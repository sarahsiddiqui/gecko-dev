/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef GMPVideoEncoderParent_h_
#define GMPVideoEncoderParent_h_

#include "mozilla/RefPtr.h"
#include "gmp-video-encode.h"
#include "mozilla/gmp/PGMPVideoEncoderParent.h"
#include "GMPMessageUtils.h"
#include "GMPSharedMemManager.h"
#include "GMPUtils.h"
#include "GMPVideoHost.h"
#include "GMPVideoEncoderProxy.h"
#include "GMPCrashHelperHolder.h"

namespace mozilla::gmp {

class GMPContentParent;

class GMPVideoEncoderParent final : public GMPVideoEncoderProxy,
                                    public PGMPVideoEncoderParent,
                                    public GMPSharedMemManager,
                                    public GMPCrashHelperHolder {
  friend class PGMPVideoEncoderParent;

 public:
  // Mark AddRef and Release as `final`, as they overload pure virtual
  // implementations in PGMPVideoEncoderParent.
  NS_INLINE_DECL_REFCOUNTING(GMPVideoEncoderParent, final)

  explicit GMPVideoEncoderParent(GMPContentParent* aPlugin);

  GMPVideoHostImpl& Host();
  void Shutdown();

  // GMPVideoEncoderProxy
  void Close() override;
  GMPErr InitEncode(const GMPVideoCodec& aCodecSettings,
                    const nsTArray<uint8_t>& aCodecSpecific,
                    GMPVideoEncoderCallbackProxy* aCallback,
                    int32_t aNumberOfCores, uint32_t aMaxPayloadSize) override;
  GMPErr Encode(GMPUniquePtr<GMPVideoi420Frame> aInputFrame,
                const nsTArray<uint8_t>& aCodecSpecificInfo,
                const nsTArray<GMPVideoFrameType>& aFrameTypes) override;
  GMPErr SetChannelParameters(uint32_t aPacketLoss, uint32_t aRTT) override;
  GMPErr SetRates(uint32_t aNewBitRate, uint32_t aFrameRate) override;
  GMPErr SetPeriodicKeyFrames(bool aEnable) override;
  uint32_t GetPluginId() const override { return mPluginId; }

  // GMPSharedMemManager
  bool MgrAllocShmem(size_t aSize, Shmem* aMem) override {
    return AllocShmem(aSize, aMem);
  }

  void MgrDeallocShmem(Shmem& aMem) override { DeallocShmem(aMem); }

 protected:
  bool MgrIsOnOwningThread() const override;

 private:
  virtual ~GMPVideoEncoderParent() = default;

  // PGMPVideoEncoderParent
  void ActorDestroy(ActorDestroyReason aWhy) override;
  mozilla::ipc::IPCResult RecvReturnShmem(ipc::Shmem&& aInputShmem) override;
  mozilla::ipc::IPCResult RecvEncodedShmem(
      const GMPVideoEncodedFrameData& aEncodedFrame, ipc::Shmem&& aEncodedShmem,
      nsTArray<uint8_t>&& aCodecSpecificInfo) override;
  mozilla::ipc::IPCResult RecvEncodedData(
      const GMPVideoEncodedFrameData& aEncodedFrame,
      nsTArray<uint8_t>&& aEncodedData,
      nsTArray<uint8_t>&& aCodecSpecificInfo) override;
  mozilla::ipc::IPCResult RecvError(const GMPErr& aError) override;
  mozilla::ipc::IPCResult RecvShutdown() override;

  size_t mEncodedShmemSize = 0;
  bool mIsOpen;
  bool mShuttingDown;
  bool mActorDestroyed;
  RefPtr<GMPContentParent> mPlugin;
  RefPtr<GMPVideoEncoderCallbackProxy> mCallback;
  GMPVideoHostImpl mVideoHost;
  const uint32_t mPluginId;
};

}  // namespace mozilla::gmp

#endif  // GMPVideoEncoderParent_h_

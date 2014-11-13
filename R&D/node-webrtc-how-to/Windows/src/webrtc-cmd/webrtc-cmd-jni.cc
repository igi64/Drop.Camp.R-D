/*
 * Copyright 2013, Pierre St Juste
 * Copyright 2014, Chris Ball
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <jni.h>

#include <iostream>
#include <string>

#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/app/webrtc/jsep.h"
#include "talk/app/webrtc/datachannelinterface.h"
#include "talk/app/webrtc/test/fakeconstraints.h"
#include "talk/app/webrtc/test/mockpeerconnectionobservers.h"
#include "webrtc/base/json.h"
#include "webrtc/base/logging.h"
#include "webrtc/base/ssladapter.h"
#include "webrtc/base/sslstreamadapter.h"
#include "webrtc/base/thread.h"
#include "talk/app/webrtc/test/fakedtlsidentityservice.h"

using rtc::scoped_ptr;
using rtc::scoped_refptr;
using webrtc::MediaStreamInterface;
using webrtc::CreatePeerConnectionFactory;
using webrtc::DataChannelInterface;
using webrtc::MockDataChannelObserver;
using webrtc::PeerConnectionFactoryInterface;
using webrtc::PeerConnectionInterface;
using webrtc::PeerConnectionObserver;
using webrtc::PortAllocatorFactoryInterface;
using webrtc::VideoSourceInterface;
using webrtc::VideoTrackInterface;

class ChatDataChannelObserver : public webrtc::DataChannelObserver {
public:
  explicit ChatDataChannelObserver(webrtc::DataChannelInterface* channel)
     : channel_(channel) {
    channel_->RegisterObserver(this);
    state_ = channel_->state();
  }
  virtual ~ChatDataChannelObserver() {
    channel_->UnregisterObserver();
  }

  virtual void OnStateChange() { state_ = channel_->state(); }
  virtual void OnMessage(const webrtc::DataBuffer& buffer) {
    std::string message_json = buffer.data.data();
    Json::Reader reader;
    Json::Value jdesc;
    if (!reader.parse(message_json, jdesc)) {
      LOG(WARNING) << "Unknown string desc " << message_json;
      return;
    }

    // replace with global constants
    std::string message = jdesc["message"].asString();
    std::cout << message << std::endl;
  }

  bool IsOpen() const { return state_ == DataChannelInterface::kOpen; }

 private:
  rtc::scoped_refptr<webrtc::DataChannelInterface> channel_;
  DataChannelInterface::DataState state_;
};

class DummySetSessionDescriptionObserver
  : public webrtc::SetSessionDescriptionObserver {
 public:
  static DummySetSessionDescriptionObserver* Create() {
    return
        new rtc::RefCountedObject<DummySetSessionDescriptionObserver>();
  }
  virtual void OnSuccess() {
    LOG(INFO) << __FUNCTION__;
  }
  virtual void OnFailure(const std::string& error) {
    LOG(INFO) << __FUNCTION__ << " " << error;
  }

 protected:
  DummySetSessionDescriptionObserver() {}
  ~DummySetSessionDescriptionObserver() {}
};

class WebRtcConnectionManager
  : public webrtc::PeerConnectionObserver,
    public webrtc::CreateSessionDescriptionObserver {
 public:
  WebRtcConnectionManager();
  bool InitConnection();
  void CreateOffer();
  void OnOfferRequest(webrtc::SessionDescriptionInterface* desc);
  void OnOfferReply(webrtc::SessionDescriptionInterface* desc);

  static webrtc::SessionDescriptionInterface* StringToDescription(
      const std::string string_desc);
  static std::string DescriptionToString(
      const webrtc::SessionDescriptionInterface* desc);

  virtual const webrtc::SessionDescriptionInterface*
      local_description() const {
    return peer_connection_->local_description();
  }
  DataChannelInterface* data_channel() { return data_channel_; }

 protected:
  //~WebRtcConnectionManager();

  // inherited from PeerConnectionObserver
  virtual void OnError();
  virtual void OnStateChange(
      webrtc::PeerConnectionObserver::StateType state_changed);
  virtual void OnAddStream(webrtc::MediaStreamInterface* stream);
  virtual void OnRemoveStream(webrtc::MediaStreamInterface* stream);
  virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate);
  virtual void OnDataChannel(webrtc::DataChannelInterface* data_channel);
  virtual void OnRenegotiationNeeded();

  // inherited from CreateSessionDescriptionObserver
  virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc);
  virtual void OnFailure(const std::string &error);
  virtual int AddRef() { return 1; }
  virtual int Release() { return 0; }
 private:
  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
      peer_connection_factory_;
  webrtc::PeerConnectionInterface::IceServers servers_;
  webrtc::PeerConnectionInterface::IceServer server_;
  webrtc::FakeConstraints constraints_;
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
  rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track_;
  rtc::scoped_refptr<webrtc::MediaStreamInterface> stream_;
  rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_;
};

const char kStunServerUri[] = "stun:stun.l.google.com:19302";
const char kAudioLabel[] = "audio_label";
const char kStreamLabel[] = "stream_label";

WebRtcConnectionManager::WebRtcConnectionManager() {
  peer_connection_factory_ = webrtc::CreatePeerConnectionFactory();
  server_.uri = kStunServerUri;
  servers_.push_back(server_);
  constraints_.SetMandatoryReceiveAudio(false);
  constraints_.SetMandatoryReceiveVideo(false);
  constraints_.AddOptional(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp, webrtc::MediaConstraintsInterface::kValueTrue);
}

bool WebRtcConnectionManager::InitConnection() {
  FakeIdentityService* dtls_service = rtc::SSLStreamAdapter::HaveDtlsSrtp() ? new FakeIdentityService() : NULL;
  peer_connection_ = peer_connection_factory_->CreatePeerConnection(servers_,        NULL, NULL, dtls_service, this);
  audio_track_ = peer_connection_factory_->CreateAudioTrack(kAudioLabel,
                                                            NULL);
  stream_ = peer_connection_factory_->CreateLocalMediaStream(kStreamLabel);
  stream_->AddTrack(audio_track_);
  peer_connection_->AddStream(stream_, &constraints_);
  data_channel_ = peer_connection_->CreateDataChannel("test1", NULL);
  data_channel_->RegisterObserver(new ChatDataChannelObserver(data_channel_));
  return true;
}

void WebRtcConnectionManager::CreateOffer() {
  peer_connection_->CreateOffer(this, &constraints_);
}

void WebRtcConnectionManager::OnOfferRequest(
    webrtc::SessionDescriptionInterface* desc) {
  peer_connection_->SetRemoteDescription(
      DummySetSessionDescriptionObserver::Create(), desc);
  peer_connection_->CreateAnswer(this, &constraints_);
}

void WebRtcConnectionManager::OnOfferReply(
    webrtc::SessionDescriptionInterface* desc) {
  peer_connection_->SetRemoteDescription(
      DummySetSessionDescriptionObserver::Create(), desc);
}

void WebRtcConnectionManager::OnError() {
  std::cout << "PEERCONNECTION ERROR" << std::endl;
}

void WebRtcConnectionManager::OnStateChange(
    webrtc::PeerConnectionObserver::StateType state_changed) {
}

void WebRtcConnectionManager::OnAddStream(
    webrtc::MediaStreamInterface* stream) {
}

void WebRtcConnectionManager::OnRemoveStream(
    webrtc::MediaStreamInterface* stream) {
}

void WebRtcConnectionManager::OnIceCandidate(
    const webrtc::IceCandidateInterface* candidate) {
}

void WebRtcConnectionManager::OnSuccess(
    webrtc::SessionDescriptionInterface* desc) {
  peer_connection_->SetLocalDescription(
      DummySetSessionDescriptionObserver::Create(), desc);
}

void WebRtcConnectionManager::OnDataChannel(
    webrtc::DataChannelInterface* data_channel) {
}

void WebRtcConnectionManager::OnRenegotiationNeeded() {
}

void WebRtcConnectionManager::OnFailure(const std::string &error) {
  std::cout << error << std::endl;
}

webrtc::SessionDescriptionInterface*
WebRtcConnectionManager::StringToDescription(const std::string string_desc) {
  Json::Reader reader;
  Json::Value jdesc;
  if (!reader.parse(string_desc, jdesc)) {
    LOG(WARNING) << "Unknown string desc " << string_desc;
    return NULL;
  }

  // replace with global constants
  std::string type = jdesc["type"].asString();
  std::string sdp = jdesc["sdp"].asString();
  return webrtc::CreateSessionDescription(type, sdp);
}

std::string WebRtcConnectionManager::DescriptionToString(
    const webrtc::SessionDescriptionInterface* desc) {
  Json::FastWriter writer;
  Json::Value jdesc;
  jdesc["type"] = desc->type();
  std::string sdp;
  desc->ToString(&sdp);
  jdesc["sdp"] = sdp;
  return writer.write(jdesc);
}

#undef JNIEXPORT
#define JNIEXPORT __attribute__((visibility("default")))
#define JOW(rettype, name) extern "C" rettype JNIEXPORT JNICALL \
          Java_com_igi64_webrtccmd_MainActivity_##name
		  
JOW(int, initCmd)(JNIEnv*, jclass) { 
  std::cout << "OK1" << std::endl;
  WebRtcConnectionManager manager;

  std::cout << "OK2" << std::endl;
  rtc::InitializeSSL(NULL);
  std::cout << "OK3" << std::endl;

  rtc::CleanupSSL();
  std::cout << "OK4" << std::endl;
}


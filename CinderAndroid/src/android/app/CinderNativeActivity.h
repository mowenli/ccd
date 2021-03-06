/*
 Copyright (c) 2015, The Cinder Project

 This code is intended to be used with the Cinder C++ library, http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#include "android/CinderAndroid.h"
#include "android/JniHelper.h"
#include "cinder/Surface.h"
#include "cinder/Url.h"

namespace cinder { namespace app {

class EventManagerAndroid;

}} // namespace cinder::app

namespace cinder { namespace android { namespace app {

/** \class CinderNativeActivity
 *
 */
class CinderNativeActivity {
public:

	CinderNativeActivity( jobject obj );
	virtual ~CinderNativeActivity();

	static CinderNativeActivity*	getInstance();
	static jclass 					getJavaClass();
	static jobject 					getJavaObject();

	static cinder::fs::path 		getCacheDirectory();
	static cinder::fs::path			getPicturesDirectory();
	static void 					setWallpaper( const Surface8u& surf );
	static int 						getDisplayRotation();
	static void 					setKeepScreenOn( bool keepScreenOn );
	static void 					setFullScreen( bool fullScreen );
	static void 					launchWebBrowser( const Url &url );
	static void 					launchTwitter( const std::string& text = "", const Surface8u* surf = nullptr );

private:
	static void 			cacheJni();
	static void 			destroyJni();

	struct Java {
		static jclassID		ClassName;
		static jclass 		ClassObject;
		static jmethodID 	getCacheDirectory;
		static jmethodID	getPicturesDirectory;
		static jmethodID 	setWallpaper;
		static jmethodID 	getDisplayRotation;
		static jmethodID 	setKeepScreenOn;
		static jmethodID 	setFullScreen;
		static jmethodID 	launchWebBrowser;
		static jmethodID 	launchTwitter;
	};

	static std::unique_ptr<CinderNativeActivity> sInstance;

	jobject					mJavaObject = nullptr;

private:
	static void registerComponents();
	static void unregisterComponents();
	friend class cinder::app::EventManagerAndroid;
};

}}} // namespace cinder::android::app
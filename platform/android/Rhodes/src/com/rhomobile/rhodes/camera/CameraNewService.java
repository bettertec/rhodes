/*------------------------------------------------------------------------
* (The MIT License)
* 
* Copyright (c) 2008-2011 Rhomobile, Inc.
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
* 
* http://rhomobile.com
*------------------------------------------------------------------------*/

package com.rhomobile.rhodes.camera;

import java.util.Iterator;
import java.util.List;

import com.rhomobile.rhodes.camera.CameraService.Size;

import android.hardware.Camera;


class CameraNewService implements CameraService {

	private static final String TAG = "CameraNewService";
	
	
	public android.hardware.Camera getMainCamera() {
		return android.hardware.Camera.open();
	}

	public android.hardware.Camera getFrontCamera() {
		// find front camera
		int camera_count = android.hardware.Camera.getNumberOfCameras();
		int i;
		for (i = 0 ; i < camera_count; i++) {
			android.hardware.Camera.CameraInfo info = new android.hardware.Camera.CameraInfo();
			android.hardware.Camera.getCameraInfo(i, info);
			if (info.facing == android.hardware.Camera.CameraInfo.CAMERA_FACING_FRONT) {
				return android.hardware.Camera.open(i);
			}
		}
		return null;
	}
	
	public Size getClosestPictureSize(android.hardware.Camera camera, int w, int h) {

		com.rhomobile.rhodes.camera.Camera.logDebug(TAG, "getClosestPictureSize("+String.valueOf(w)+", "+String.valueOf(h)+")");
		
		int neww = w;
		int newh = h;
		
		Camera.Parameters p = camera.getParameters();
		if (p == null) {
			com.rhomobile.rhodes.camera.Camera.logDebug(TAG, "getClosestPictureSize() return null 1");
			return null;
		}
		List<android.hardware.Camera.Size> sizes = p.getSupportedPictureSizes();
		if (sizes == null) {
			com.rhomobile.rhodes.camera.Camera.logDebug(TAG, "getClosestPictureSize() return null 2");
			return null;
		}
		Iterator<android.hardware.Camera.Size> iter = sizes.iterator();
		if (iter == null) {
			com.rhomobile.rhodes.camera.Camera.logDebug(TAG, "getClosestPictureSize() return null 3");
			return null;
		}
		// find closest preview size
		float min_r = -1;
		int minW = 0;
		int minH = 0;
		com.rhomobile.rhodes.camera.Camera.logDebug(TAG, "     enumerate camera sizes :");
		while (iter.hasNext()) {
			android.hardware.Camera.Size s = iter.next();
			com.rhomobile.rhodes.camera.Camera.logDebug(TAG, "        - ["+String.valueOf(s.width)+"x"+String.valueOf(s.height)+"]");
			if (min_r < 0) {
				min_r = (float)s.width*(float)s.width+(float)s.height*(float)s.height;
				minW = s.width;
				minH = s.height;
			}
			else {
				float cur_r = ((float)neww-(float)s.width)*((float)neww-(float)s.width)+((float)newh-(float)s.height)*((float)newh-(float)s.height);
				if (cur_r < min_r) {
					min_r = cur_r;
					minW = s.width;
					minH = s.height;
				}
			}
		}
		if (min_r >= 0) {
			neww = minW;
			newh = minH;
		}
		else {
			com.rhomobile.rhodes.camera.Camera.logDebug(TAG, "getClosestPictureSize() return null 4");
			return null;
		}
		com.rhomobile.rhodes.camera.Camera.logDebug(TAG, "getClosestPictureSize() return ["+String.valueOf(neww)+"x"+String.valueOf(newh)+"]");
		return new Size(neww, newh);
	}
	
	public Size getClosestPreviewSize(android.hardware.Camera camera, int w, int h) {

		com.rhomobile.rhodes.camera.Camera.logDebug(TAG, "getClosestPreviewSize("+String.valueOf(w)+", "+String.valueOf(h)+")");
		int neww = w;
		int newh = h;
		
		Camera.Parameters p = camera.getParameters();
		if (p == null) {
			com.rhomobile.rhodes.camera.Camera.logDebug(TAG, "getClosestPreviewSize() return null - Camera do not return Parameters");
			return null;
		}
		List<android.hardware.Camera.Size> sizes = p.getSupportedPreviewSizes();
		if (sizes == null) {
			com.rhomobile.rhodes.camera.Camera.logDebug(TAG, "getClosestPreviewSize() return null - Camera do not return supportedPreviewSize");
			return null;
		}
		Iterator<android.hardware.Camera.Size> iter = sizes.iterator();
		if (iter == null) {
			com.rhomobile.rhodes.camera.Camera.logDebug(TAG, "getClosestPreviewSize() return null - Iterator is null");
			return null;
		}
		// find closest preview size
		float min_r = -1;
		int minW = 0;
		int minH = 0;
		while (iter.hasNext()) {
			android.hardware.Camera.Size s = iter.next();
			com.rhomobile.rhodes.camera.Camera.logDebug(TAG, "        enumerate Size: "+String.valueOf(s.width)+", "+String.valueOf(s.height));
			if (min_r < 0) {
				min_r = (float)s.width*(float)s.width+(float)s.height*(float)s.height;
				minW = s.width;
				minH = s.height;
			}
			else {
				float cur_r = ((float)neww-(float)s.width)*((float)neww-(float)s.width)+((float)newh-(float)s.height)*((float)newh-(float)s.height);
				if (cur_r < min_r) {
					min_r = cur_r;
					minW = s.width;
					minH = s.height;
				}
			}
		}
		if (min_r >= 0) {
			neww = minW;
			newh = minH;
		}
		else {
			com.rhomobile.rhodes.camera.Camera.logDebug(TAG, "getClosestPreviewSize() return null - do not found size");
			return null;
		}
		com.rhomobile.rhodes.camera.Camera.logDebug(TAG, "getClosestPreviewSize() return ["+String.valueOf(neww)+", "+String.valueOf(newh)+"]");
		return new Size(neww, newh);
	}
	
	public boolean isAutoFocusSupported(android.hardware.Camera camera) {
		String focus_mode = camera.getParameters().getFocusMode();
		boolean auto_focus_supported = false;
		if  ( focus_mode != null ) {
			auto_focus_supported = (focus_mode.equals(android.hardware.Camera.Parameters.FOCUS_MODE_AUTO)) || (focus_mode.equals(android.hardware.Camera.Parameters.FOCUS_MODE_MACRO));
		}
		return auto_focus_supported;
	}
	

}
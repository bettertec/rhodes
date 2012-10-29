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

package com.rhomobile.rhodes.mapview;

import java.io.IOException;
import java.util.List;
import java.util.Map;
import java.util.Vector;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.Drawable;
import android.location.Address;
import android.location.Geocoder;
import android.os.Bundle;
import android.os.IBinder;
import android.view.ViewGroup.LayoutParams;
import android.widget.RelativeLayout;

import com.google.android.maps.GeoPoint;
import com.google.android.maps.MapActivity;
import com.google.android.maps.MapController;
import com.google.android.maps.MyLocationOverlay;
import com.rhomobile.rhodes.AndroidR;
import com.rhomobile.rhodes.BaseActivity;
import com.rhomobile.rhodes.Logger;
import com.rhomobile.rhodes.RhoConf;
import com.rhomobile.rhodes.RhodesActivity;
import com.rhomobile.rhodes.RhodesService;
import com.rhomobile.rhodes.util.PerformOnUiThread;

public class GoogleMapView extends MapActivity {

	private static final String TAG = "GoogleMapView";
	
	private static final String SETTINGS_PREFIX = RhodesService.INTENT_EXTRA_PREFIX + ".settings.";
	private static final String ANNOTATIONS_PREFIX = RhodesService.INTENT_EXTRA_PREFIX + ".annotations.";
	
	private static GoogleMapView mc = null;
	
	private ServiceConnection mServiceConnection = null;
	
	private com.google.android.maps.MapView view;
	private AnnotationsOverlay annOverlay;
	private CalloutOverlay mCalloutOverlay;
	private MyLocationOverlay mMyLocationOverlay;
	
	private double spanLat = 0;
	private double spanLon = 0;
	
	private String apiKey;
	
	private Vector<Annotation> annotations;
	
	static private ExtrasHolder mHolder = null;
	
	private static class Coordinates {
		public double latitude;
		public double longitude;
		
		public Coordinates() {
			latitude = 0;
			longitude = 0;
		}
	};
	
	private Coordinates center = new Coordinates();
	
	private static void reportFail(String name, Exception e) {
		Logger.E(TAG, "Call of \"" + name + "\" failed: " + e.getMessage());
	}
	
	@Override
	public void onDestroy() {
		if (mServiceConnection != null) {
			unbindService(mServiceConnection);
			mServiceConnection = null;
		}
		super.onDestroy();
		mc = null;
	}
	
	public void selectAnnotation(Annotation ann) {
		final Annotation fann = ann;
		PerformOnUiThread.exec(new Runnable() {
			public void run() {
				mCalloutOverlay.selectAnnotation(fann);
			}
		}, false);
	}

	@Override
	public void onCreate(Bundle icicle) {
		super.onCreate(icicle);
		
		Intent intent = new Intent(this, RhodesService.class);
		mServiceConnection = new ServiceConnection() {
			@Override
			public void onServiceDisconnected(ComponentName name) {}
			@Override
			public void onServiceConnected(ComponentName name, IBinder service) {}
		};
		bindService(intent, mServiceConnection, Context.BIND_AUTO_CREATE);
		
		mc = this;
		
		getWindow().setFlags(RhodesService.WINDOW_FLAGS, RhodesService.WINDOW_MASK);
		
		RelativeLayout layout = new RelativeLayout(this);
		setContentView(layout, new LayoutParams(LayoutParams.FILL_PARENT, LayoutParams.FILL_PARENT));
		
		// Extrace parameters
		//Bundle extras = getIntent().getExtras();
		
		ExtrasHolder extras = mHolder;
		
		apiKey = extras.getString(SETTINGS_PREFIX + "api_key");
		
		// Extract settings
		String map_type = extras.getString(SETTINGS_PREFIX + "map_type");
		if (map_type == null)
			map_type = "roadmap";
		
		boolean zoom_enabled = extras.getBoolean(SETTINGS_PREFIX + "zoom_enabled");
		//boolean scroll_enabled = extras.getBoolean(SETTINGS_PREFIX + "scroll_enabled");
		boolean shows_user_location = extras.getBoolean(SETTINGS_PREFIX + "shows_user_location");
		
		// Extract annotations
		int size = extras.getInt(ANNOTATIONS_PREFIX + "size") + 1;
		annotations = new Vector<Annotation>(size);
		for (int i = 0; i < size; ++i) {
			Annotation ann = new Annotation();
			String prefix = ANNOTATIONS_PREFIX + Integer.toString(i) + ".";
			
			ann.latitude = 10000;
			ann.longitude = 10000;
			
			String lat = extras.getString(prefix + "latitude");
			if (lat != null) {
				try {
					ann.latitude = Double.parseDouble(lat);
				}
				catch (NumberFormatException e) {}
			}
			
			String lon = extras.getString(prefix + "longitude");
			if (lon != null) {
				try {
					ann.longitude = Double.parseDouble(lon);
				}
				catch (NumberFormatException e) {}
			}
			
			ann.type = "ann";
			ann.address = extras.getString(prefix + "address");
			ann.title = extras.getString(prefix + "title");
			ann.subtitle = extras.getString(prefix + "subtitle");
			ann.url = extras.getString(prefix + "url");
			if (ann.url != null)
				ann.url = RhodesService.getInstance().normalizeUrl(ann.url);
			
			ann.image = extras.getString(prefix+"image");
			ann.image_x_offset = extras.getInt(prefix + "image_x_offset");
			ann.image_y_offset = extras.getInt(prefix + "image_y_offset");
			
			annotations.addElement(ann);
		}
		
		// Create view
		view = new com.google.android.maps.MapView(this, apiKey);
		view.setClickable(true);
		layout.addView(view);
		
		Bitmap pin = BitmapFactory.decodeResource(getResources(), AndroidR.drawable.marker);
		
		Drawable marker = getResources().getDrawable(AndroidR.drawable.marker);
		marker.setBounds(0, 0, marker.getIntrinsicWidth(), marker.getIntrinsicHeight());
		annOverlay = new AnnotationsOverlay(this, marker, pin.getDensity());
		
		mCalloutOverlay = new CalloutOverlay(this, marker);
		
		if (shows_user_location) {
			mMyLocationOverlay = new MyLocationOverlay(this, view);
			view.getOverlays().add(mMyLocationOverlay);
		}

		view.getOverlays().add(annOverlay);
		view.getOverlays().add(mCalloutOverlay);
		
		
		// Apply extracted parameters
		view.setBuiltInZoomControls(zoom_enabled);
		view.setSatellite(map_type.equals("hybrid") || map_type.equals("satellite"));
		view.setTraffic(false);
		
		MapController controller = view.getController();
		String type = extras.getString(SETTINGS_PREFIX + "region");
		if (type.equals("square")) {
			String latitude = extras.getString(SETTINGS_PREFIX + "region.latitude");
			String longitude = extras.getString(SETTINGS_PREFIX + "region.longitude");
			if (latitude != null && longitude != null) {
				try {
					double lat = Double.parseDouble(latitude);
					double lon = Double.parseDouble(longitude);
					center.latitude = lat;
					center.longitude = lon;
					controller.setCenter(new GeoPoint((int)(lat*1000000), (int)(lon*1000000)));
				}
				catch (NumberFormatException e) {
					Logger.E(TAG, "Wrong region center: " + e.getMessage());
				}
			}
			
			String latSpan = extras.getString(SETTINGS_PREFIX + "region.latSpan");
			String lonSpan = extras.getString(SETTINGS_PREFIX + "region.lonSpan");
			if (latSpan != null && lonSpan != null) {
				try {
					double lat = Double.parseDouble(latSpan);
					double lon = Double.parseDouble(lonSpan);
					controller.zoomToSpan((int)(lat*1000000), (int)(lon*1000000));
				}
				catch (NumberFormatException e) {
					Logger.E(TAG, "Wrong region span: " + e.getMessage());
				}
			}
		}
		else if (type.equals("circle")) {
			String center = extras.getString(SETTINGS_PREFIX + "region.center");
			String radius = extras.getString(SETTINGS_PREFIX + "region.radius");
			if (center != null && radius != null) {
				try {
					double span = Double.parseDouble(radius);
					spanLat = spanLon = span;
					Annotation ann = new Annotation();
					ann.type = "center";
					ann.latitude = ann.longitude = 10000;
					ann.address = center;
					annotations.insertElementAt(ann, 0);
				}
				catch (NumberFormatException e) {
					Logger.E(TAG, "Wrong region radius: " + e.getMessage());
				}
			}
		}
		
		//mHolder.clear();

		view.preLoad();
		
		Thread geocoding = new Thread(new Runnable() {
			public void run() {
				doGeocoding();
			}
		});
		geocoding.start();

        if (RhoConf.getBool("disable_screen_rotation")) 
            setRequestedOrientation(BaseActivity.getScreenProperties().getOrientation());
	}
	
	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		Logger.T(TAG, "+++ onConfigurationChanged");
		if (RhoConf.getBool("disable_screen_rotation"))
		{
			super.onConfigurationChanged(newConfig);
			setRequestedOrientation(BaseActivity.getScreenProperties().getOrientation());
		}
		else
		{
            super.onConfigurationChanged(newConfig);
            BaseActivity.getScreenProperties().reread(this);
		}
	}
	
	
	
	@Override
	protected void onStart() {
		super.onStart();
		RhodesService.activityStarted();
	}
	
	@Override
	protected void onResume() {
		super.onResume();
		if (mMyLocationOverlay != null)
			mMyLocationOverlay.enableMyLocation();

	}
	
	@Override
	protected void onPause() {
		super.onPause();
		if (mMyLocationOverlay != null)
			mMyLocationOverlay.disableMyLocation();

	}
	
	@Override
	protected void onStop() {
		RhodesService.activityStopped();
		super.onStop();
	}
	
	private void doGeocoding() {
		Vector<Annotation> anns = new Vector<Annotation>();
		
		Context context = RhodesActivity.getContext();
		
		for (int i = 0, lim = annotations.size(); i < lim; ++i) {
			Annotation ann = annotations.elementAt(i);
			if (ann.latitude == 10000 || ann.longitude == 10000)
				continue;
			anns.addElement(ann);
		}
		
		for (int i = 0, lim = annotations.size(); i < lim; ++i) {
			Annotation ann = annotations.elementAt(i);
			if (ann.latitude != 10000 && ann.longitude != 10000)
				continue;
			if (ann.address == null)
				continue;
			
			Geocoder gc = new Geocoder(context);
			try {
				List<Address> addrs = gc.getFromLocationName(ann.address, 1);
				if (addrs.size() == 0)
					continue;
				
				Address addr = addrs.get(0);
				
				ann.latitude = addr.getLatitude();
				ann.longitude = addr.getLongitude();
				if (ann.type.equals("center")) {
					MapController controller = view.getController();
					center.latitude = ann.latitude;
					center.longitude = ann.longitude;
					controller.setCenter(new GeoPoint((int)(ann.latitude*1000000), (int)(ann.longitude*1000000)));
					controller.zoomToSpan((int)(spanLat*1000000), (int)(spanLon*1000000));
					PerformOnUiThread.exec(new Runnable() {
						public void run() {
							view.invalidate();
						}
					}, false);
				}
				else
					anns.addElement(ann);
			} catch (IOException e) {
				Logger.E(TAG, "GeoCoding request failed: " + e.getMessage());
			}
			
		}
		addAnnotationsInUIThread(annOverlay, anns, view);
		
		PerformOnUiThread.exec(new Runnable() {
			public void run() {
				view.invalidate();
			}
		}, false);
	}
	
	private class AddAnnotationsCommand implements Runnable {
		public AddAnnotationsCommand(AnnotationsOverlay overlay, Vector<Annotation> annotations, com.google.android.maps.MapView view) {
			mOverlay = overlay;
			mAnnotations = annotations;
			mView = view;
		}
		public void run() {
			//Utils.platformLog(TAG, "add Annotation !");
			mOverlay.addAnnotations(mAnnotations);
			mView.invalidate();
		}
		private AnnotationsOverlay mOverlay;
		private Vector<Annotation> mAnnotations;
		private com.google.android.maps.MapView mView;
	}
	
	private void addAnnotationsInUIThread(AnnotationsOverlay overlay, Vector<Annotation> annotations, com.google.android.maps.MapView view) {
		//Utils.platformLog(TAG, "perform add Annotations !");
		PerformOnUiThread.exec(new AddAnnotationsCommand(overlay, annotations, view), false);
	}

	@Override
	protected boolean isRouteDisplayed() {
		return false;
	}
	
	@SuppressWarnings("unchecked")
	public static void create(String gapiKey, Map<String, Object> params) {
		mHolder = new ExtrasHolder();
		try {
			Intent intent_obj = new Intent(RhodesActivity.getContext(), GoogleMapView.class);
			mHolder.clear();
			ExtrasHolder intent = mHolder;
			intent.putExtra(SETTINGS_PREFIX + "api_key", gapiKey);
			
			Object settings = params.get("settings");
			if (settings != null && (settings instanceof Map<?,?>)) {
				Map<Object, Object> hash = (Map<Object, Object>)settings;
				Object map_type = hash.get("map_type");
				if (map_type != null && (map_type instanceof String))
					intent.putExtra(SETTINGS_PREFIX + "map_type", (String)map_type);
				
				Object zoom_enabled = hash.get("zoom_enabled");
				if (zoom_enabled != null && (zoom_enabled instanceof String))
					intent.putExtra(SETTINGS_PREFIX + "zoom_enabled", ((String)zoom_enabled).equalsIgnoreCase("true"));
				
				Object scroll_enabled = hash.get("scroll_enabled");
				if (scroll_enabled != null && (scroll_enabled instanceof String))
					intent.putExtra(SETTINGS_PREFIX + "scroll_enabled", ((String)scroll_enabled).equalsIgnoreCase("true"));
				
				Object shows_user_location = hash.get("shows_user_location");
				if (shows_user_location != null && (shows_user_location instanceof String))
					intent.putExtra(SETTINGS_PREFIX + "shows_user_location", ((String)shows_user_location).equalsIgnoreCase("true"));
				
				Object region = hash.get("region");
				if (region != null) {
					if (region instanceof Vector<?>) {
						Vector<String> reg = (Vector<String>)region;
						if (reg.size() == 4) {
							intent.putExtra(SETTINGS_PREFIX + "region", "square");
							intent.putExtra(SETTINGS_PREFIX + "region.latitude", reg.elementAt(0));
							intent.putExtra(SETTINGS_PREFIX + "region.longitude", reg.elementAt(1));
							intent.putExtra(SETTINGS_PREFIX + "region.latSpan", reg.elementAt(2));
							intent.putExtra(SETTINGS_PREFIX + "region.lonSpan", reg.elementAt(3));
						}
					}
					else if (region instanceof Map<?,?>) {
						Map<Object, Object> reg = (Map<Object,Object>)region;
						String center = null;
						String radius = null;
						
						Object centerObj = reg.get("center");
						if (centerObj != null && (centerObj instanceof String))
							center = (String)centerObj;
						
						Object radiusObj = reg.get("radius");
						if (radiusObj != null && (radiusObj instanceof String))
							radius = (String)radiusObj;
						
						if (center != null && radius != null) {
							intent.putExtra(SETTINGS_PREFIX + "region", "circle");
							intent.putExtra(SETTINGS_PREFIX + "region.center", center);
							intent.putExtra(SETTINGS_PREFIX + "region.radius", radius);
						}
					}
				}
			}
			
			Object annotations = params.get("annotations");
			if (annotations != null && (annotations instanceof Vector<?>)) {
				Vector<Object> arr = (Vector<Object>)annotations;
				
				intent.putExtra(ANNOTATIONS_PREFIX + "size", arr.size());
				
				for (int i = 0, lim = arr.size(); i < lim; ++i) {
					Object annObj = arr.elementAt(i);
					if (annObj == null || !(annObj instanceof Map<?, ?>))
						continue;
					
					Map<Object, Object> ann = (Map<Object, Object>)annObj;
					
					String prefix = ANNOTATIONS_PREFIX + Integer.toString(i) + ".";
					
					Object latitude = ann.get("latitude");
					if (latitude != null && (latitude instanceof String))
						intent.putExtra(prefix + "latitude", (String)latitude);
					
					Object longitude = ann.get("longitude");
					if (longitude != null && (longitude instanceof String))
						intent.putExtra(prefix + "longitude", (String)longitude);
					
					Object address = ann.get("street_address");
					if (address != null && (address instanceof String))
						intent.putExtra(prefix + "address", (String)address);
					
					Object title = ann.get("title");
					if (title != null && (title instanceof String))
						intent.putExtra(prefix + "title", (String)title);
					
					Object subtitle = ann.get("subtitle");
					if (subtitle != null && (subtitle instanceof String))
						intent.putExtra(prefix + "subtitle", (String)subtitle);
					
					Object url = ann.get("url");
					if (url != null && (url instanceof String))
						intent.putExtra(prefix + "url", (String)url);

					Object image = ann.get("image");
					if (image != null && (image instanceof String))
						intent.putExtra(prefix + "image", (String)image);

					Object image_x_offset = ann.get("image_x_offset");
					if (image_x_offset != null && (image_x_offset instanceof String))
						intent.putExtra(prefix + "image_x_offset", (String)image_x_offset);
					
					Object image_y_offset = ann.get("image_y_offset");
					if (image_y_offset != null && (image_y_offset instanceof String))
						intent.putExtra(prefix + "image_y_offset", (String)image_y_offset);
				}
			}
			
			RhodesService.getInstance().startActivity(intent_obj);
		}
		catch (Exception e) {
			reportFail("create", e);
		}
	}
	
	public static void close() {
		try {
			PerformOnUiThread.exec(new Runnable() {
				public void run() {
					if (mc != null) {
						mc.finish();
						mc = null;
					}
				}
			}, false);
		}
		catch (Exception e) {
			reportFail("close", e);
		}
	}
	
	public static boolean isStarted() {
		return mc != null;
	}
	
	public static double getCenterLatitude() {
		try {
			if (mc == null)
				return 0;
			return mc.center.latitude;
		}
		catch (Exception e) {
			reportFail("getCenterLatitude", e);
			return 0;
		}
	}
	
	public static double getCenterLongitude() {
		try {
			if (mc == null)
				return 0;
			return mc.center.longitude;
		}
		catch (Exception e) {
			reportFail("getCenterLongitude", e);
			return 0;
		}
	}
}

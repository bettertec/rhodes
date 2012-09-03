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

#import "Rhodes.h"
#import "Bluetooth.h"
#import "AppManager.h"
#import "common/RhodesApp.h"
#include "ruby/ext/rho/rhoruby.h"



#define BTC_OK  "OK"
#define BTC_CANCEL  "CANCEL"
#define BTC_ERROR  "ERROR"
#define BTC_NOT_FOUND  "NOT_FOUND"

#define BT_OK  @BTC_OK
#define BT_CANCEL  @BTC_CANCEL
#define BT_ERROR  @BTC_ERROR

#define BT_ROLE_SERVER  @"ROLE_SERVER"
#define BT_ROLE_CLIENT  @"ROLE_CLIENT"

#define BT_SESSION_INPUT_DATA_RECEIVED @"SESSION_INPUT_DATA_RECEIVED"
#define BT_SESSION_DISCONNECT @"SESSION_DISCONNECT"

// session id must be equal on each device
#define BT_RHOMOBILE_SESSION_ID @"RhomobileBluetoothSession"


#define kMaxPacketSize 1024


@implementation RhoBluetoothManager


static RhoBluetoothManager *instance = NULL;

+ (RhoBluetoothManager*)sharedInstance {
	if (instance == NULL) {
		instance = [[RhoBluetoothManager alloc] init];
        instance.mode = PEER_MODE;
        instance.custom_connect_client_name = nil;
        instance.custom_connect_server_name = nil;
		instance.mysession = nil;
		instance.deviceName = [[UIDevice currentDevice] name];
		instance.connectedDeviceName = nil;
		instance.connectedDeviceID = nil;
		instance.connectionCallbackURL = nil;
		instance.sessionCallbackURL = nil;
		instance.packets = [NSMutableArray arrayWithCapacity:128];
	}
    return instance;
}

- (void)dealloc { 
	[self invalidateSession:self.mysession];
	self.mysession = nil;
	self.deviceName = nil;
	self.connectedDeviceName = nil;
	
	[super dealloc];
}


@synthesize mysession, deviceName, connectedDeviceName, connectedDeviceID, connectionCallbackURL, sessionCallbackURL, packets, mode, custom_connect_client_name, custom_connect_server_name;


#pragma mark -
#pragma mark Peer Picker Related Methods

-(void)startPicker:(NSString*)role {
	GKPeerPickerController*		picker;
	picker = [[GKPeerPickerController alloc] init]; // note: picker is released in various picker delegate methods when picker use is done.
	picker.delegate = self;
	[picker show]; // show the Peer Picker
}

#pragma mark GKPeerPickerControllerDelegate Methods

- (void)peerPickerControllerDidCancel:(GKPeerPickerController *)picker { 
	// Peer Picker automatically dismisses on user cancel. No need to programmatically dismiss.
    
	// autorelease the picker. 
	picker.delegate = nil;
    [picker autorelease]; 
	
	// invalidate and release game session if one is around.
	if(self.mysession != nil)	{
		[self invalidateSession:self.mysession];
		self.mysession = nil;
	}
	[self fireConnectionCallback:BT_CANCEL connected_device_name:@""];
} 

- (GKSession *)peerPickerController:(GKPeerPickerController *)picker sessionForConnectionType:(GKPeerPickerConnectionType)type { 
	// session id - identifier for session
	// display name - name for display on other devices
	
	GKSession *msession = [[GKSession alloc] initWithSessionID:BT_RHOMOBILE_SESSION_ID displayName:deviceName sessionMode:GKSessionModePeer]; 
	return [msession autorelease]; // peer picker retains a reference, so autorelease ours so we don't leak.
}

- (void)peerPickerController:(GKPeerPickerController *)picker didConnectPeer:(NSString *)peerID toSession:(GKSession *)csession { 
	// Remember the current peer.
	self.connectedDeviceID = peerID;
	self.connectedDeviceName = [csession displayNameForPeer:peerID];  // copy
	
	// Make sure we have a reference to the game session and it is set up
	self.mysession = csession; // retain
	self.mysession.delegate = self; 
	[self.mysession setDataReceiveHandler:self withContext:NULL];
	
	// Done with the Peer Picker so dismiss it.
	[picker dismiss];
	picker.delegate = nil;
	[picker autorelease];
	
	[self fireConnectionCallback:BT_OK connected_device_name:self.connectedDeviceName];

} 


#pragma mark -
#pragma mark Session Related Methods

- (void)invalidateSession:(GKSession *)msession {
	if(msession != nil) {
		[msession disconnectFromAllPeers]; 
		msession.available = NO; 
		[msession setDataReceiveHandler: nil withContext: NULL]; 
		msession.delegate = nil; 
	}
}



#pragma mark Data Send/Receive Methods

- (void)receiveData:(NSData *)data fromPeer:(NSString *)peer inSession:(GKSession *)session context:(void *)context { 
	static int lastPacketTime = -1;

	unsigned char *incomingPacket = (unsigned char *)[data bytes];
	int incomingPacketSize= [data length];
	
	[self addToPackets:incomingPacket length:incomingPacketSize];
	[self fireSessionCallback:connectedDeviceName event_type:BT_SESSION_INPUT_DATA_RECEIVED];
}

- (void)sendData:(GKSession *)session withData:(void *)data ofLength:(int)length reliable:(BOOL)howtosend {
	// the packet we'll send is resued
	static unsigned char networkPacket[kMaxPacketSize];
	
	unsigned char* indata = (unsigned char*)data;
	int size = length;
	while (size > 0) {
		int send_size = size;
		if (send_size > kMaxPacketSize) {
			send_size = kMaxPacketSize;
		}
		memcpy( &networkPacket[0], indata, send_size ); 
		NSData *packet = [NSData dataWithBytes: networkPacket length: (length)];
		if(howtosend == YES) { 
			//[self.session sendData:packet toPeers:[NSArray arrayWithObject:self.connectedDeviceID] withDataMode:GKSendDataReliable error:nil];
			[self.mysession sendDataToAllPeers:packet withDataMode:GKSendDataReliable error:nil];
		} else {
			//[self.session sendData:packet toPeers:[NSArray arrayWithObject:self.connectedDeviceID] withDataMode:GKSendDataUnreliable error:nil];
			[self.mysession sendDataToAllPeers:packet withDataMode:GKSendDataUnreliable error:nil];
		}
		size -= send_size;
		indata += send_size;
	}
}

- (void)sendNSData:(NSData*)cdata {
	[self sendData:self.mysession withData:[cdata bytes] ofLength:[cdata length] reliable:YES];
}

#pragma mark GKSessionDelegate Methods

// we've gotten a state change in the session
- (void)session:(GKSession *)session peer:(NSString *)peerID didChangeState:(GKPeerConnectionState)state { 
	
	if(state == GKPeerStateDisconnected) {
		// We've been disconnected from the other peer.
		[self fireSessionCallback:connectedDeviceName event_type:BT_SESSION_DISCONNECT];
		
	} 
    if ((state == GKPeerStateConnected) && ((self.mode == CLIENT_MODE) || (self.mode == SERVER_MODE))) {
        self.connectedDeviceID = peerID;
        self.connectedDeviceName = [session displayNameForPeer:peerID];  // copy
        self.mode = CONNECTED_MODE;
        //TODO: callback
        [self fireConnectionCallback:BT_OK connected_device_name:self.connectedDeviceName];
    }
    
    if ((state == GKPeerStateAvailable) && ((self.mode == CLIENT_MODE))) {
        BOOL accept = NO;
        NSString* device_name = [session displayNameForPeer:peerID];
        if (self.custom_connect_server_name != nil) {
            if ([self.custom_connect_server_name compare:device_name] == NSOrderedSame) {
                accept = YES;
            }
        }
        if (accept) {
            [session connectToPeer:peerID withTimeout:20];
        }
    }
} 

- (void)session:(GKSession *)session didReceiveConnectionRequestFromPeer:(NSString *)peerID {
    NSError* error;
    BOOL accept = YES;
    //NSString* device_name = [session displayNameForPeer:peerID];
    //if (self.custom_connect_client_name != nil) {
    //    if (!([self.custom_connect_client_name compare:device_name] == NSOrderedSame)) {
    //        accept = NO;
    //    }
    //}
    if (accept) {
        [session acceptConnectionFromPeer:peerID error:&error];
    }
    else {
        [session denyConnectionFromPeer:peerID];
    }
}

- (void)fireConnectionCallback:(NSString*)status connected_device_name:(NSString*)connected_device_name {
	if (connectionCallbackURL == nil) {
		return;
	}
	NSString* strBody = @"";
	strBody = [strBody stringByAppendingString:@"&status="];
	strBody = [strBody stringByAppendingString:status];
	strBody = [strBody stringByAppendingString:@"&connected_device_name="];
	strBody = [strBody stringByAppendingString:connected_device_name];
    char* norm_url = rho_http_normalizeurl([connectionCallbackURL UTF8String]);
	rho_net_request_with_data(norm_url, [strBody UTF8String]);
    rho_http_free(norm_url);
}

- (void)fireSessionCallback:(NSString*)connected_device_name event_type:(NSString*)event_type {
	if (sessionCallbackURL == nil) {
		return;
	}
	NSString* strBody = @"";
	strBody = [strBody stringByAppendingString:@"&connected_device_name="];
	strBody = [strBody stringByAppendingString:connected_device_name];
	strBody = [strBody stringByAppendingString:@"&event_type="];
	strBody = [strBody stringByAppendingString:event_type];
    char* norm_url = rho_http_normalizeurl([sessionCallbackURL UTF8String]);
	rho_net_request_with_data(norm_url, [strBody UTF8String]);
    rho_http_free(norm_url);
}


- (void)startConnect:(NSString*)callback {
    self.mode = PEER_MODE;
	self.mysession = nil;
	self.connectedDeviceName = nil;
	self.connectedDeviceID = nil;
	
	self.connectionCallbackURL = callback;
	[self performSelectorOnMainThread:@selector(startPicker:) withObject:BT_ROLE_SERVER waitUntilDone:NO];
}

-(void)startClientConnection:(NSString*)role {
    self.mode = CLIENT_MODE;
    self.mysession = [[GKSession alloc] initWithSessionID:BT_RHOMOBILE_SESSION_ID displayName:deviceName sessionMode:GKSessionModeClient];
    self.mysession.delegate = self;
    [self.mysession setDataReceiveHandler:self withContext:NULL];
    self.mysession.available = YES;
}

-(void)startServerConnection:(NSString*)role {
    self.mode = SERVER_MODE;
    self.mysession = [[GKSession alloc] initWithSessionID:BT_RHOMOBILE_SESSION_ID displayName:deviceName sessionMode:GKSessionModeServer];
    self.mysession.delegate = self;
    [self.mysession setDataReceiveHandler:self withContext:NULL];
    self.mysession.available = YES;
}

- (void)stop_current_connection {
    if (((self.mode == CLIENT_MODE) || (self.mode == SERVER_MODE))) {
        if (self.mysession != nil) {
            [self invalidateSession:self.mysession]; 
            self.mysession = nil;
        }
        self.mode = PEER_MODE;
        [self fireConnectionCallback:BT_CANCEL connected_device_name:@""];
    }
}

- (void)startServerConnect:(NSString*)client_name callback:(NSString*)callback {
	self.mysession = nil;
	self.connectedDeviceName = nil;
	self.connectedDeviceID = nil;
    self.custom_connect_client_name = client_name;
	self.connectionCallbackURL = callback;
	[self performSelectorOnMainThread:@selector(startServerConnection:) withObject:BT_ROLE_CLIENT waitUntilDone:NO];
}

- (void)startClientConnect:(NSString*)server_name callback:(NSString*)callback {
	self.mysession = nil;
	self.connectedDeviceName = nil;
	self.connectedDeviceID = nil;
	self.connectionCallbackURL = callback;
    self.custom_connect_server_name = server_name;
	[self performSelectorOnMainThread:@selector(startClientConnection:) withObject:BT_ROLE_SERVER waitUntilDone:NO];
}


- (int)readFromPackets:(void*)buf length:(int)length {
	unsigned char *dst = (unsigned char *)buf;
	int packets_size = [self getPacketsSize];
	int readed_size = 0;
	while ((readed_size < length) && (readed_size < packets_size) && ([self.packets count] > 0)) {
		NSData* data = [self.packets objectAtIndex:0];
		if ([data length] <= (length-readed_size)) {
			int to_read = [data length];
			// read data and remove packet
			[data getBytes:dst length:to_read];
			dst+= to_read;
			readed_size += to_read;
			[self.packets removeObjectAtIndex:0];
		}
		else {
			// read part of packet + reduce packet size
			int to_read = length-readed_size;
			[data getBytes:dst length:to_read];
			dst+= to_read;
			readed_size += to_read;
			NSData* udata = [data subdataWithRange:NSMakeRange(to_read, [data length]-to_read)];
			[self.packets removeObjectAtIndex:0];
			[self.packets insertObject:udata atIndex:0];
		}
	}
	return readed_size;
}

- (void)addToPackets:(void*)buf length:(int)length {
	NSData* data = [NSData dataWithBytes:buf length:length];
	[self.packets addObject:data];
}

- (void)clearPackets {
	[self.packets removeAllObjects];
}

- (int)getPacketsSize {
	int size =0;
	int i;
	for (i = 0; i < self.packets.count; i++) {
		NSData* data = [self.packets objectAtIndex:i];
		size += [data length];
	}
	return size;
}

- (NSString*)readString {
	int length = 0;
	[self readFromPackets:&length length:4];
	char* buf = (char*)malloc(length+2);
	[self readFromPackets:buf length:length];
	buf[length] = 0;
	buf[length+1] = 0;
	NSString* s = [NSString stringWithUTF8String:buf];
	free(buf);
	return s;
}

- (void)sendString:(NSString*)string {
	if (self.mysession == nil) {
		return;
	}
	const char* sbuf = [string UTF8String];
	int length = strlen(sbuf);
	
	char* buf = malloc(length+5);
	char* dst = buf;
	*((int*)dst) = length;
	dst += 4;
	memcpy(dst, sbuf, length);
	
	//[self sendData:self.session withData:(void*)buf ofLength:(4+length) reliable:YES];
	NSData* data = [NSData dataWithBytes:buf length:(4+length)];
	[self performSelectorOnMainThread:@selector(sendNSData:) withObject:data waitUntilDone:NO];
	free(buf);
}

- (void)sendData:(void*)buf length:(int)length {
	if (self.mysession == nil) {
		return;
	}
	//[self sendData:self.session withData:buf ofLength:length reliable:YES];
	NSData* data = [NSData dataWithBytes:buf length:(length)];
	[self performSelectorOnMainThread:@selector(sendNSData:) withObject:data waitUntilDone:NO];
}

- (void)doDisconnectCommand:(NSString*)object {
	if (self.mysession == nil) {
		return;
	}
	[self invalidateSession:self.mysession];
	self.mysession = nil;
	[self fireSessionCallback:connectedDeviceName event_type:BT_SESSION_DISCONNECT];
	self.sessionCallbackURL = nil;
}

- (void)doDisconnect {
	[self performSelectorOnMainThread:@selector(doDisconnectCommand:) withObject:nil waitUntilDone:NO];
}

- (void)doBluetoothOffCommand:(NSString*)object {
	[self doDisconnectCommand:nil];
	instance = NULL;
}

- (void)doBluetoothOff {
	[self performSelectorOnMainThread:@selector(doBluetoothOffCommand:) withObject:nil waitUntilDone:NO];
}

@end



int rho_bluetooth_is_bluetooth_available() {
	return 1;
}

void rho_bluetooth_off_bluetooth() {
	[[RhoBluetoothManager sharedInstance] doBluetoothOff];
}

void rho_bluetooth_set_device_name(const char* device_name) {
	NSString* newname = [NSString stringWithUTF8String:device_name];
	[RhoBluetoothManager sharedInstance].deviceName = newname;
}

VALUE rho_bluetooth_get_device_name() {
	return rho_ruby_create_string([[RhoBluetoothManager sharedInstance].deviceName UTF8String]);
}

const char* rho_bluetooth_get_last_error() {
	return BTC_OK;
}

const char* rho_bluetooth_create_session(const char* role, const char* callback_url) {
	//NSString* role = [NSString stringWithUTF8String:role];
	NSString* callback = [NSString stringWithUTF8String:callback_url];
	[[RhoBluetoothManager sharedInstance] startConnect:callback];
	return BTC_OK;
}


const char* rho_bluetooth_create_custom_server_session(const char* client_name, const char* callback_url, int accept_any_device) {
	NSString* ns_callback = [NSString stringWithUTF8String:callback_url];
	NSString* ns_client_name = nil;
    if (!accept_any_device) {
        ns_client_name = [NSString stringWithUTF8String:client_name];
    }
	[[RhoBluetoothManager sharedInstance] startServerConnect:ns_client_name callback:ns_callback];
	return BTC_OK;
}

const char* rho_bluetooth_create_custom_client_session(const char* server_name, const char* callback_url) {
	NSString* ns_callback = [NSString stringWithUTF8String:callback_url];
	NSString* ns_server_name = [NSString stringWithUTF8String:server_name];
	[[RhoBluetoothManager sharedInstance] startClientConnect:ns_server_name callback:ns_callback];
	return BTC_OK;
}

const char* rho_bluetooth_stop_current_connection_process() {
    [[RhoBluetoothManager sharedInstance] stop_current_connection];
	return BTC_OK;
}



void rho_bluetooth_session_set_callback(const char* connected_device_name, const char* callback_url) {
	NSString* callback = [NSString stringWithUTF8String:callback_url];
	[RhoBluetoothManager sharedInstance].sessionCallbackURL = callback;
}

void rho_bluetooth_session_disconnect(const char* connected_device_name) {
	[[RhoBluetoothManager sharedInstance] doDisconnect];
}

int rho_bluetooth_session_get_status(const char* connected_device_name) {
	return [[RhoBluetoothManager sharedInstance] getPacketsSize]; 
}

VALUE rho_bluetooth_session_read_string(const char* connected_device_name) {
	NSString* s = [[RhoBluetoothManager sharedInstance] readString];
	return rho_ruby_create_string([s UTF8String]);
}

void rho_bluetooth_session_write_string(const char* connected_device_name, const char* str) {
	NSString* s = [NSString stringWithUTF8String:str];
	[[RhoBluetoothManager sharedInstance] sendString:s];
}

VALUE rho_bluetooth_session_read_data(const char* connected_device_name) {
	int size = [[RhoBluetoothManager sharedInstance] getPacketsSize];
	unsigned char* buf = malloc(size);
	[[RhoBluetoothManager sharedInstance] readFromPackets:buf length:size];
	VALUE val = rho_ruby_create_byte_array(buf, size);
	free(buf);
	return val;
}

void rho_bluetooth_session_write_data(const char* connected_device_name, VALUE data) {
	int size = rho_ruby_unpack_byte_array(data, 0, 0);
	if (size <= 0) {
		return;
	}
	unsigned char* buf = malloc(size);
	size = rho_ruby_unpack_byte_array(data, buf, size);
	[[RhoBluetoothManager sharedInstance] sendData:buf length:size];
	free(buf);
}





//
//  pigeon.h
//  pigeon
//
//  Created by gamebase on 20/8/16.
//  Copyright © 2016年 gamebase. All rights reserved.
//
#ifndef __pigeon_define_h
#define __pigeon_define_h

enum State {
    stReady,
	stClosed,
	stConnecting,
	stConnected,
	stEstablishing,
	stEstablished,
	stBroken,  // connection is broken, will automatically try re-connecting
	stBroken2, // connection is broken, won't try re-connecting automatically
	stClosing,

	stTestStarted = 100,
	stTestWaitResp,
	stTestStopped,
};

enum SvrErrors {
	SvrErrors_CannotFindSession = 1, //服务端没找到对应的sessionId
	SvrErrors_AuthFailure = 2, //服务端验证sessionId失败
	SvrErrors_CRCCheckFailed = 3, //CRC校验失败
	SvrErrors_UnknownAlgorithms = 4, //无效的加密算法
	SvrErrors_UnknownCompressAlgorithms = 5, //"无效的压缩算法"),
	SvrErrors_Request_ID_Error = 6, //"RequestId错误"),
	SvrErrors_InvalidSession = 7, //服务端没找到对应的sessionId
};

/************************************************************************/
/* 业务层错误                                                           */
/************************************************************************/
enum DataErrorType {
	DataError_NO_ERROR = 0,
	DataError_UNKNOWN_ERROR = -1,		 //未知错误
	DataError_INVALID_OP_CODE = 1,     //无效的opcode
	DataError_DECOMPRESS_ERROR = 2,    //解压缩错误
	DataError_DECODE_ERROR = 3,        //  解码错误
	DataError_CHECK_PARAM_ERROR = 4,   // 校验参数错误
};

enum ErrorTypes {
	// error happens in socket (low-level errors)
	ErrorTypes_ServerClosed = 1,	//服务器关闭，建议客户端返回登录页面
	ErrorTypes_ConnectFailed = 2,	//客户端重连或者返回登录页面
	ErrorTypes_ConnectBroken = 3,	//C++网络层通过一次repair后，仍然失败返回
	ErrorTypes_WriteError = 4,		//
	ErrorTypes_ReadError = 5,
	ErrorTypes_CRCCheckFailed = 6,	//服务器crc校验失败
	ErrorTypes_SessionError = 7,	//session错误，建议客户端重新登录
	ErrorTypes_TimeOut = 8,			//超时
	ErrorTypes_RequestIdError = 9,	//请求id非递增
	// protocol errors
	ErrorTypes_Inactive = 100,		// long time in idle 心跳超时
	ErrorTypes_EstablishError = 101, //登录验证过程中发生错误
};

enum RetLuaError {
    Error_ServerClosed = 4000,
	Error_ConnectFailed = 4001,
	Error_ConnectBroken = 4002,
//    Error_ConnectReset = 4003, //需要重连
//    Error_Inactive = 4004,
    Error_SessionError = 4005,
	Error_ReqTimedout = 4100,
	Error_FatalError = 4999,
};

#endif

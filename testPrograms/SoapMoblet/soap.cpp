/* Copyright (C) 2009 Mobile Sorcery AB

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.
*/

//system headers
#include <MAUtil/util.h>
#include <conprint.h>
#include <maassert.h>

//local headers
#include "soap.h"

//static const data
static const char sSoapHeader[] =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
"<soap12:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
"xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
"xmlns:soap12=\"http://www.w3.org/2003/05/soap-envelope\">"
"<soap12:Body>\n";
static const int sSoapHeaderLength = sizeof(sSoapHeader) - 1;
static const char sSoapFooter[] =
"</soap12:Body>"
"</soap12:Envelope>\n";
static const int sSoapFooterLength = sizeof(sSoapFooter) - 1;


SoapRequest::SoapRequest() : mHttp(this), mState(eIdle) {
}
SoapRequest::~SoapRequest() {
	cancel();
}

int SoapRequest::start(const char* url, const void* data, int dataLen, SoapListener* listener) {
	MAASSERT(mState == eIdle);
	int res = mHttp.create(url, HTTP_POST);
	if(res < 0) {
		return res;
	}

	char buf[32];
	sprintf(buf, "%i", sSoapHeaderLength + dataLen + sSoapFooterLength);
	mHttp.setRequestHeader("Content-Length", buf);

	mHttp.setRequestHeader("Content-Type", "application/soap+xml; charset=utf-8");

	mHttp.write(sSoapHeader, sSoapHeaderLength);
	mState = eWritingHeader;
	mListener = listener;
	mData = data;
	mDataLen = dataLen;
	return 0;
}

void SoapRequest::cancel() {
	if(mState != eIdle)
		mHttp.close();
	mState = eIdle;
}

void SoapRequest::connWriteFinished(Connection* conn, int result) {
	MAASSERT(conn == &mHttp);
	if(result < 0) {
		mListener->soapError(result);
		return;
	}
	switch(mState) {
	case eWritingHeader:
		mHttp.write(mData, mDataLen);
		mState = eWritingData;
		break;
	case eWritingData:
		mHttp.write(sSoapFooter, sSoapFooterLength);
		mState = eWritingFooter;
		break;
	case eWritingFooter:
		mHttp.finish();
		mState = eFinishing;
		break;
	default:
		BIG_PHAT_ERROR;
	}
}

void SoapRequest::httpFinished(HttpConnection* http, int result) {
	MAASSERT(http == &mHttp);
	if(result < 200 || result >= 300) {
		mListener->soapError(result);
		return;
	}
	MAASSERT(mState == eFinishing);

	printf("recv\n");
	mResponsePtr = mResponseBuffer;
	mHttp.recv(mResponseBuffer, BUFLEN - 1);	// -1 for null terminator
	mState = eReading;

	mContext.init(this);
	mInBody = false;
}

void SoapRequest::connRecvFinished(Connection* conn, int result) {
	printf("connRecvFinished %i\n", result);
	MAASSERT(conn == &mHttp);
	if(result < 0) {
		mListener->soapError(result);
		return;
	}

	mResponsePtr[result] = 0;
	mResponsePtr = mResponseBuffer;
	mContext.feed(mResponseBuffer);
	if(mState == eReading)
		mHttp.recv(mResponsePtr, BUFLEN - 1 - (mResponsePtr - mResponseBuffer));
}

//******************************************************************************
// MtxListener
//******************************************************************************

//#define DEBUG
#ifdef DEBUG
#define LOG printf
#else
#define LOG(...)
#endif

void SoapRequest::mtxEncoding(const char* name) {
	LOG("Encoding %i\n", name);
	if(mInBody)
		mListener->mtxEncoding(name);
}

void SoapRequest::mtxDataRemains(const char* data, int len) {
	if(mResponseBuffer != data) {	//valid value: don't memcpy.
		memcpy(mResponseBuffer, data, len);
	}
	mResponsePtr = mResponseBuffer + len;
}
void SoapRequest::mtxParseError() {
	LOG("parseError\n");
	if(mInBody)
		mListener->mtxParseError();
	mState = eError;
}
void SoapRequest::mtxTagStart(const char* name, int len) {
	LOG("TagStart %i %s\n", len, name);
	if(mInBody)
		mListener->mtxTagStart(name, len);
	if(strcmp(name, "soap:Body") == 0)
		mInBody = true;
}
void SoapRequest::mtxTagAttr(const char* attrName, const char* attrValue) {
	LOG("TagAttrib %s %s\n", attrName, attrValue);
	if(mInBody)
		mListener->mtxTagAttr(attrName, attrValue);
}
void SoapRequest::mtxTagData(const char* data, int len) {
	LOG("TagData %i \"%s\"\n", len, data);
	if(mInBody)
		mListener->mtxTagData(data, len);
}
void SoapRequest::mtxTagEnd(const char* name, int len) {
	LOG("TagEnd %i \"%s\"\n", len, name);
	if(strcmp(name, "soap:Body") == 0) {
		mInBody = false;
		mListener->soapEnd();
	}
	if(mInBody)
		mListener->mtxTagEnd(name, len);
}
void SoapRequest::mtxEmptyTagEnd() {
	LOG("EmptyTagEnd\n");
	if(mInBody)
		mListener->mtxEmptyTagEnd();
}

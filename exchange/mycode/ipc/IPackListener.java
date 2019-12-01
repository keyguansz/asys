/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Original file: frameworks/base/core/java/android/app/v1/IPackListener.aidl
 */
package android.app.v1;
/**
 * {@hide}
 */
public interface IPackListener extends android.os.IInterface
{
/** Local-side IPC implementation stub class. */
public static abstract class Stub extends android.os.Binder implements android.app.v1.IPackListener
{
private static final java.lang.String DESCRIPTOR = "android.app.v1.IPackListener";
/** Construct the stub at attach it to the interface. */
public Stub()
{
this.attachInterface(this, DESCRIPTOR);
}
/**
 * Cast an IBinder object into an android.app.v1.IPackListener interface,
 * generating a proxy if needed.
 */
public static android.app.v1.IPackListener asInterface(android.os.IBinder obj)
{
if ((obj==null)) {
return null;
}
android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
if (((iin!=null)&&(iin instanceof android.app.v1.IPackListener))) {
return ((android.app.v1.IPackListener)iin);
}
return new android.app.v1.IPackListener.Stub.Proxy(obj);
}
@Override public android.os.IBinder asBinder()
{
return this;
}
@Override public boolean onTransact(int code, android.os.Parcel data, android.os.Parcel reply, int flags) throws android.os.RemoteException
{
switch (code)
{
case INTERFACE_TRANSACTION:
{
reply.writeString(DESCRIPTOR);
return true;
}
case TRANSACTION_onRecv:
{
data.enforceInterface(DESCRIPTOR);
android.app.v1.Pack _arg0;
if ((0!=data.readInt())) {
_arg0 = android.app.v1.Pack.CREATOR.createFromParcel(data);
}
else {
_arg0 = null;
}
this.onRecv(_arg0);
reply.writeNoException();
return true;
}
}
return super.onTransact(code, data, reply, flags);
}
private static class Proxy implements android.app.v1.IPackListener
{
private android.os.IBinder mRemote;
Proxy(android.os.IBinder remote)
{
mRemote = remote;
}
@Override public android.os.IBinder asBinder()
{
return mRemote;
}
public java.lang.String getInterfaceDescriptor()
{
return DESCRIPTOR;
}
@Override public void onRecv(android.app.v1.Pack pack) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
try {
_data.writeInterfaceToken(DESCRIPTOR);
if ((pack!=null)) {
_data.writeInt(1);
pack.writeToParcel(_data, 0);
}
else {
_data.writeInt(0);
}
mRemote.transact(Stub.TRANSACTION_onRecv, _data, _reply, 0);
_reply.readException();
}
finally {
_reply.recycle();
_data.recycle();
}
}
}
static final int TRANSACTION_onRecv = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
}
public void onRecv(android.app.v1.Pack pack) throws android.os.RemoteException;
}

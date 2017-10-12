// TODO word
# framework debug 技巧
## 安装
   拷贝文件到system/framework/;
   stop;
   start重启system_server进程

## Java调试
其实整个调试过程非常简单：
  - 打断点
  - 跟踪代码（Step in/out/over等等）

### 打断点
    在正确的进程的合适位置打断点:区别于普通app的进程，framwork的所在进程需要进行源码分析，比如ams是运行在system_server中的，并且这些进程只能在root的机子调试（模拟器或者root真机）
### 跟踪代码
### 打印日志
    -系统一般都会有debug标志，我们修改源码中的标志，打印就好，比如
    修改后

    -- Log.e("tag",logStr);

## native调试

## ref
- [如何调试Android Framework？](http://weishu.me/2016/05/30/how-to-debug-android-framework/)
- [Android Studio如何调试Framework层的代码？](https://www.zhihu.com/question/37606394)
- [Markdown 编辑器推荐](https://github.com/wizardforcel/markdown-simple-world/blob/master/1.md)

# pms
不论是cmd安装，还是预装market安装，还是ui安装，最终都会调用到installPackage这个方法入口，本节单独讨论系统是如何执行这一个过程的
## 总流程
代码调用流程如下：
``` java
├── PMS.installPackage()
    └── PMS.installPackageAsUser()
         |传递 InstallParams 参数
        PackageHandler.doHandleMessage().INIT_COPY
         |
        PackageHandler.doHandleMessage().MCS_BOUND
         ├── HandlerParams.startCopy()
         │    ├── InstallParams.handleStartCopy()
         │    │    └──InstallArgs.copyApk()
         │    └── InstallParams.handleReturnCode()
         │         └── PMS.processPendingInstall()
         │              ├── InstallArgs.doPreInstall()
         │              ├── PMS.installPackageLI()
         │              │    ├── PackageParser.parsePackage()
         │              │    ├── PackageParser.collectCertificates()
         │              │    ├── PackageParser.collectManifestDigest()
         │              │    ├── PackageDexOptimizer.performDexOpt()
         │              │    ├── InstallArgs.doRename()
         │              │    │    └── InstallArgs.getNextCodePath()
         │              │    ├── replacePackageLI()
         │              │    │    ├── shouldCheckUpgradeKeySetLP()
         │              │    │    ├── compareSignatures()
         │              │    │    ├── replaceSystemPackageLI()
         │              │    │    │    ├── killApplication()
         │              │    │    │    ├── removePackageLI()
         │              │    │    │    ├── Settings.disableSystemPackageLPw()
         │              │    │    │    ├── createInstallArgsForExisting()
         │              │    │    │    ├── deleteCodeCacheDirsLI()
         │              │    │    │    ├── scanPackageLI()
         │              │    │    │    └── updateSettingsLI()
         │              │    │    └── replaceNonSystemPackageLI()
         │              │    │         ├── deletePackageLI()
         │              │    │         ├── deleteCodeCacheDirsLI()
         │              │    │         ├── scanPackageLI()
         │              │    │         └── updateSettingsLI()
         │              │    └── installNewPackageLI()
         │              │         ├── scanPackageLI()
         │              │         └── updateSettingsLI()
         │              ├── InstallArgs.doPostInstall()
         │              ├── BackupManager.restoreAtInstall()
         │              └── sendMessage(POST_INSTALL)
         │                   |
         │                  PackageHandler.doHandleMessage().POST_INSTALL
         │                   ├── grantRequestedRuntimePermissions()
         │                   ├── sendPackageBroadcast()
         │                   └── IPackageInstallObserver.onPackageInstalled()
         └── PackageHandler.doHandleMessage().MCS_UNBIND
              └── PackageHandler.disconnectService()
```
## 文件拷贝阶段
### installPackage
installPackage方法只是用当前用户安装应用，最后也会调用installPackageAsUser
//TODO 用户的概念？一个进程就是一个用户？
```java
@Override
public void installPackage(String originPath, IPackageInstallObserver2 observer,int installFlags, String installerPackageName, VerificationParams verificationParams,String packageAbiOverride) {
    installPackageAsUser(originPath, observer, installFlags,installerPackageName, verificationParams,packageAbiOverride, UserHandle.getCallingUserId());
}
```

### installPackageAsUser
installPackageAsUser先检查调用进程是否有安装应用的权限，[再检查调用进程所属的用户是否有权限安装应用](为何需要检查用户?)，最后检查指定的用户是否被限制安装应用。如果参数installFlags带有INSTALL_ALL_USERS，则该应用将给系统中所有用户安装，否则只给指定用户安装。安装应用实践比较长，因此不可能在一个函数中完成。上面函数把数据保存在installParams然后发送了INIT_COPY消息。通过PackageHandler的实例mHandler.sendMessage（msg）把信息发给继承Handler的类HandleMessage()方法会自动调用Packagemanager的安装方法installPackage（），发送消息时会传递一个InstallParams参数，InstallParams是继承自HandlerParams抽象类的，用来记录安装应用的参数。
<!-- FIXME 则该应用将给系统中所有用户安装,但是目前的Android一般都是单用户吧？-->
<!-- TODO 会存在线程堵塞问题么 -->
```java
@Override
    public void installPackageAsUser(String originPath, IPackageInstallObserver2 observer,
            int installFlags, String installerPackageName, VerificationParams verificationParams,
            String packageAbiOverride, int userId) {
        //检查调用进程的权限,比如PackageInstaller.apk这个系统应用就必须申请这个权限
        mContext.enforceCallingOrSelfPermission(android.Manifest.permission.INSTALL_PACKAGES, null);
	     //检查调用进程的用户是否有权限安装应用
        final int callingUid = Binder.getCallingUid();
        enforceCrossUserPermission(callingUid, userId, true, true, "installPackageAsUser");
	     //检查指定的用户是否被限制安装应用
       // TODO DISALLOW_INSTALL_APPS 是安装黑名单
        if (isUserRestricted(userId, UserManager.DISALLOW_INSTALL_APPS)) {
            try {
                if (observer != null) {
                    observer.onPackageInstalled("", INSTALL_FAILED_USER_RESTRICTED, null, null);
                }
            } catch (RemoteException re) {
            }
            return;
        }
        //adb INSTALL_FAILED_USER_RESTRICTED
        if ((callingUid == Process.SHELL_UID) || (callingUid == Process.ROOT_UID)) {
            installFlags |= PackageManager.INSTALL_FROM_ADB;
        } else {
            // Caller holds INSTALL_PACKAGES permission, so we're less strict
            // about installerPackageName.

            installFlags &= ~PackageManager.INSTALL_FROM_ADB;
            installFlags &= ~PackageManager.INSTALL_ALL_USERS;
        }
        //给所有用户安装
        UserHandle user;
        if ((installFlags & PackageManager.INSTALL_ALL_USERS) != 0) {
            user = UserHandle.ALL;
        } else {
            user = new UserHandle(userId);
        }

        verificationParams.setInstallerUid(callingUid);

        final File originFile = new File(originPath);
        final OriginInfo origin = OriginInfo.fromUntrustedFile(originFile);
        //保存参数到InstallParamsm,发送消息
        final Message msg = mHandler.obtainMessage(INIT_COPY);
        msg.obj = new InstallParams(origin, observer, installFlags,
                installerPackageName, verificationParams, user, packageAbiOverride);
        mHandler.sendMessage(msg);
    }
```

### doHandleMessage-INIT_COPY

```java
void doHandleMessage(Message msg) {
            switch (msg.what) {
                case INIT_COPY: {
                    HandlerParams params = (HandlerParams) msg.obj;
                    int idx = mPendingInstalls.size();
                    if (DEBUG_INSTALL) Slog.i(TAG, "init_copy idx=" + idx + ": " + params);
                    // If a bind was already initiated we dont really
                    // need to do anything. The pending install
                    // will be processed later on.
                    if (!mBound) {
                        // If this is the only one pending we might
                        // have to bind to the service again.
                        if (!connectToService()) {//绑定DefaultContainerService
                            Slog.e(TAG, "Failed to bind to media container service");
                            params.serviceError();
                            return;
                        } else {//连接成功把安装信息保存到mPendingInstalls
                            // Once we bind to the service, the first
                            // pending request will be processed.
                            mPendingInstalls.add(idx, params);
                        }
                    } else {//如果已经绑定好了
                        mPendingInstalls.add(idx, params);
                        // Already bound to the service. Just make
                        // sure we trigger off processing the first request.
                        if (idx == 0) {
                            mHandler.sendEmptyMessage(MCS_BOUND);
                        }
                    }
                    break;
                }
```
INIT_COPY消息的处理将绑定DefaultContainerService，因为这是一个异步的过程，要等待绑定的结果通过onServiceConnected返回，所以这里的安装参数放到了mPendingInstalls列表中。如果这个Service以前就绑定好了，现在就不需要再绑定，安装信息也会先放到mPendingInstalls。如果有多个安装请求同时到达，这里通过mPendingInstalls列表对他们进行排队。如果列表中只有一项，说明没有更多的安装请求，因此这种情况下回立即发出MCS_BOUND消息。而onServiceConnected方法同样是发出MCS_BOUND消息：
```java
class DefaultContainerConnection implements ServiceConnection {
        public void onServiceConnected(ComponentName name, IBinder service) {
            if (DEBUG_SD_INSTALL) Log.i(TAG, "onServiceConnected");
            IMediaContainerService imcs =
                IMediaContainerService.Stub.asInterface(service);
            mHandler.sendMessage(mHandler.obtainMessage(MCS_BOUND, imcs));
        }

        public void onServiceDisconnected(ComponentName name) {
            if (DEBUG_SD_INSTALL) Log.i(TAG, "onServiceDisconnected");
        }
    };
```
看下MCS_BOUND的消息处理
```java
case MCS_BOUND: {
                    if (DEBUG_INSTALL) Slog.i(TAG, "mcs_bound");
                    if (msg.obj != null) {
                        mContainerService = (IMediaContainerService) msg.obj;
                    }
                    if (mContainerService == null) {//没有连接成功
                        // Something seriously wrong. Bail out
                        Slog.e(TAG, "Cannot bind to media container service");
                        for (HandlerParams params : mPendingInstalls) {
                            // Indicate service bind error
                            params.serviceError();//通知出错了
                        }
                        mPendingInstalls.clear();
                    } else if (mPendingInstalls.size() > 0) {
                        HandlerParams params = mPendingInstalls.get(0);
                        if (params != null) {
                            if (params.startCopy()) {//执行安装
                                // We are done...  look for more work or to
                                // go idle.
                                if (DEBUG_SD_INSTALL) Log.i(TAG,
                                        "Checking for more work or unbind...");
                                // Delete pending install
                                if (mPendingInstalls.size() > 0) {
                                    mPendingInstalls.remove(0);//工作完成，删除第一项
                                }
                                if (mPendingInstalls.size() == 0) {//如果没有安装消息了，延时发送10秒MCS_UNBIND消息
                                    if (mBound) {
                                        if (DEBUG_SD_INSTALL) Log.i(TAG,
                                                "Posting delayed MCS_UNBIND");
                                        removeMessages(MCS_UNBIND);
                                        Message ubmsg = obtainMessage(MCS_UNBIND);
                                        // Unbind after a little delay, to avoid
                                        // continual thrashing.
                                        sendMessageDelayed(ubmsg, 10000);
                                    }
                                } else {
                                    // There are more pending requests in queue.
                                    // Just post MCS_BOUND message to trigger processing
                                    // of next pending install.
                                    if (DEBUG_SD_INSTALL) Log.i(TAG,
                                            "Posting MCS_BOUND for next work");
                                    mHandler.sendEmptyMessage(MCS_BOUND);//还有消息继续发送MCS_BOUND消息
                                }
                            }
                        }
                    } else {
                        // Should never happen ideally.
                        Slog.w(TAG, "Empty queue");
                    }
                    break;
                }
```
如果结束了我们看看MCS_UNBIND消息的处理
```java
case MCS_UNBIND: {
                    // If there is no actual work left, then time to unbind.
                    if (DEBUG_INSTALL) Slog.i(TAG, "mcs_unbind");

                    if (mPendingInstalls.size() == 0 && mPendingVerification.size() == 0) {
                        if (mBound) {
                            if (DEBUG_INSTALL) Slog.i(TAG, "calling disconnectService()");

                            disconnectService();//断开连接
                        }
                    } else if (mPendingInstalls.size() > 0) {
                        // There are more pending requests in queue.
                        // Just post MCS_BOUND message to trigger processing
                        // of next pending install.
                        mHandler.sendEmptyMessage(MCS_BOUND);
                    }

                    break;
                }
```
MCS_UNBIND消息的处理，如果处理的时候发现mPendingInstalls又有数据了，还是发送MCS_BOUND消息继续安装，否则断开和DefaultContainerService的连接，安装结束。这个安装会尝试4次，超过4次就GG了
下面我们看执行安装的函数startCopy：
```java
final boolean startCopy() {
            boolean res;
            try {
                if (DEBUG_INSTALL) Slog.i(TAG, "startCopy " + mUser + ": " + this);

                if (++mRetries > MAX_RETRIES) {//重试超过4次退出
                    Slog.w(TAG, "Failed to invoke remote methods on default container service. Giving up");
                    mHandler.sendEmptyMessage(MCS_GIVE_UP);
                    handleServiceError();
                    return false;
                } else {
                    handleStartCopy();
                    res = true;
                }
            } catch (RemoteException e) {
                if (DEBUG_INSTALL) Slog.i(TAG, "Posting install MCS_RECONNECT");
                mHandler.sendEmptyMessage(MCS_RECONNECT);//安装出错，发送重新连接
                res = false;
            }
            handleReturnCode();
            return res;
        }
```
### InstallParams.handleStartCopy
handleStartCopy()执行的工作如下：

- 判断安装标志位是否合法
- 判断安装空间是否足够
- 对安装位置的校验
- 判断是否需要对应用进行校验工作
- 如果校验成功，执行InstallArgs.copyApk()
- 如果无需校验，直接执行InstallArgs.copyApk()

handleStartCopy函数先通过DefaultContainerService调用了getMinimallPackageInfo来确定安装位置是否有足够的空间，并在PackageInfoLite对象的recommendedIntallLocation记录错误原因。发现空间不够，会调用installer的freecache方法来释放一部分空间。
// 首先对安装的标志位进行判断，如果既有内部安装标志，又有外部安装标志，那么就设置
   //PackageManager.INSTALL_FAILED_INVALID_INSTALL_LOCATION返回值
再接下来handleStartCopy有很长一段都在处理apk的校验，这个校验过程是通过发送Intent ACTION_PACKAGE_NEEDS_VERIFICATION给系统中所有接受该Intent的应用来完成。如果无需校验，直接调用InstallArgs对象的copyApk方法。

这个方法比较长，分段来看。
``` java
ret = PackageManager.INSTALL_SUCCEEDED
final StorageManager storage = StorageManager.from(mContext);
final long lowThreshold = storage.getStorageLowBytes(
        Environment.getDataDirectory());
final long sizeBytes = mContainerService.calculateInstalledSize(
        origin.resolvedPath, isForwardLocked(), packageAbiOverride);
if (mInstaller.freeCache(null, sizeBytes + lowThreshold) >= 0) {
    pkgLite = mContainerService.getMinimalPackageInfo(origin.resolvedPath,
            installFlags, packageAbiOverride);
}
```
首先，如果需要的空间不够大，就调用Install的freeCache去释放一部分缓存。这里的mContainerService对应的binder服务端实现，在DefaultContainerService中。中间经过复杂（安装位置，pkgLite.recommendedIntallLocation，安装位置的校验，installLocationPoliy策略等）的判断处理之后，创建一个InstallArgs对象，如果前面的判断结果是能安装成功的话ret=PackageManager.INSTALL_SUCCEEDED，进入分支。
//  TODO installLocationPoliy() 是位置的策略PackageINfoLite
``` java
if (ret == PackageManager.INSTALL_SUCCEEDED) {
                 /*
                 * ADB installs appear as UserHandle.USER_ALL, and can only be performed by
                 * UserHandle.USER_OWNER, so use the package verifier for UserHandle.USER_OWNER.
                 */
                int userIdentifier = getUser().getIdentifier();
                if (userIdentifier == UserHandle.USER_ALL
                        && ((installFlags & PackageManager.INSTALL_FROM_ADB) != 0)) {
                    userIdentifier = UserHandle.USER_OWNER;
                }
                /*
                 * Determine if we have any installed package verifiers. If we
                 * do, then we'll defer to them to verify the packages.
                 */
                final int requiredUid = mRequiredVerifierPackage == null ? -1
                        : getPackageUid(mRequiredVerifierPackage, userIdentifier);
                if (!origin.existing && requiredUid != -1
                        && isVerificationEnabled(userIdentifier, installFlags)) {
                    final Intent verification = new Intent(
                            Intent.ACTION_PACKAGE_NEEDS_VERIFICATION);
                    verification.addFlags(Intent.FLAG_RECEIVER_FOREGROUND);
                    verification.setDataAndType(Uri.fromFile(new File(origin.resolvedPath)),
                            PACKAGE_MIME_TYPE);
                    verification.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                    final List<ResolveInfo> receivers = queryIntentReceivers(verification,
                            PACKAGE_MIME_TYPE, PackageManager.GET_DISABLED_COMPONENTS,
                            0 /* TODO: Which userId? */);
                    if (DEBUG_VERIFY) {
                        Slog.d(TAG, "Found " + receivers.size() + " verifiers for intent "
                                + verification.toString() + " with " + pkgLite.verifiers.length
                                + " optional verifiers");
                    }
                    final int verificationId = mPendingVerificationToken++;
                    verification.putExtra(PackageManager.EXTRA_VERIFICATION_ID, verificationId);
                    verification.putExtra(PackageManager.EXTRA_VERIFICATION_INSTALLER_PACKAGE,
                            installerPackageName);
                    verification.putExtra(PackageManager.EXTRA_VERIFICATION_INSTALL_FLAGS,
                            installFlags);
                    verification.putExtra(PackageManager.EXTRA_VERIFICATION_PACKAGE_NAME,
                            pkgLite.packageName);
                    verification.putExtra(PackageManager.EXTRA_VERIFICATION_VERSION_CODE,
                            pkgLite.versionCode);
                    if (verificationParams != null) {
                        if (verificationParams.getVerificationURI() != null) {
                           verification.putExtra(PackageManager.EXTRA_VERIFICATION_URI,
                                 verificationParams.getVerificationURI());
                        }
                        if (verificationParams.getOriginatingURI() != null) {
                            verification.putExtra(Intent.EXTRA_ORIGINATING_URI,
                                  verificationParams.getOriginatingURI());
                        }
                        if (verificationParams.getReferrer() != null) {
                            verification.putExtra(Intent.EXTRA_REFERRER,
                                  verificationParams.getReferrer());
                        }
                        if (verificationParams.getOriginatingUid() >= 0) {
                            verification.putExtra(Intent.EXTRA_ORIGINATING_UID,
                                  verificationParams.getOriginatingUid());
                        }
                        if (verificationParams.getInstallerUid() >= 0) {
                            verification.putExtra(PackageManager.EXTRA_VERIFICATION_INSTALLER_UID,
                                  verificationParams.getInstallerUid());
                        }
                    }
                    final PackageVerificationState verificationState = new PackageVerificationState(
                            requiredUid, args);
                    mPendingVerification.append(verificationId, verificationState);
                    final List<ComponentName> sufficientVerifiers = matchVerifiers(pkgLite,
                            receivers, verificationState);
                    // Apps installed for "all" users use the device owner to verify the app
                    UserHandle verifierUser = getUser();
                    if (verifierUser == UserHandle.ALL) {
                        verifierUser = UserHandle.OWNER;
                    }
                    /*
                     * If any sufficient verifiers were listed in the package
                     * manifest, attempt to ask them.
                     */
                    if (sufficientVerifiers != null) {
                        final int N = sufficientVerifiers.size();
                        if (N == 0) {
                            Slog.i(TAG, "Additional verifiers required, but none installed.");
                            ret = PackageManager.INSTALL_FAILED_VERIFICATION_FAILURE;
                        } else {
                            for (int i = 0; i < N; i++) {
                                final ComponentName verifierComponent = sufficientVerifiers.get(i);
                                final Intent sufficientIntent = new Intent(verification);
                                sufficientIntent.setComponent(verifierComponent);
                                mContext.sendBroadcastAsUser(sufficientIntent, verifierUser);
                            }
                        }
                    }
                    final ComponentName requiredVerifierComponent = matchComponentForVerifier(
                            mRequiredVerifierPackage, receivers);
                    if (ret == PackageManager.INSTALL_SUCCEEDED
                            && mRequiredVerifierPackage != null) {
                        /*
                         * Send the intent to the required verification agent,
                         * but only start the verification timeout after the
                         * target BroadcastReceivers have run.
                         */
                        verification.setComponent(requiredVerifierComponent);
                        mContext.sendOrderedBroadcastAsUser(verification, verifierUser,
                                android.Manifest.permission.PACKAGE_VERIFICATION_AGENT,
                                new BroadcastReceiver() {
                                    @Override
                                    public void onReceive(Context context, Intent intent) {
                                        final Message msg = mHandler
                                                .obtainMessage(CHECK_PENDING_VERIFICATION);
                                        msg.arg1 = verificationId;
                                        mHandler.sendMessageDelayed(msg, getVerificationTimeout());
                                    }
                                }, null, 0, null, null);
                        /*
                         * We don't want the copy to proceed until verification
                         * succeeds, so null out this field.
                         */
                        mArgs = null;
                    }
                } else {
                    /*
                     * No package verification is enabled, so immediately start
                     * the remote call to initiate copy using temporary file.
                     */
                    ret = args.copyApk(mContainerService, true);
                }
            }

```
InstallArgs是个抽象类，一共有三个实现类MoveInstallArgs（针对已有文件的Move）、AsecInstallArgs（针对SD卡）和FileInstallArgs（针对内部存储），会在createInstallArgs()方法中根据不同的参数返回不同的实现类。
接下来分析FileInstallArgs.copyApk()方法：
###  FileInstallArgs.copyApk()
``` java
int copyApk(IMediaContainerService imcs, boolean temp) throws RemoteException {
    // 已经执行过copy了
    if (origin.staged) {
        codeFile = origin.file;
        resourceFile = origin.file;
        return PackageManager.INSTALL_SUCCEEDED;
    }
    try {
        // 在/data/app/下面生成一个类似vmdl1354353418.tmp的临时文件
        final File tempDir = mInstallerService.allocateStageDirLegacy(volumeUuid);
        codeFile = tempDir;
        resourceFile = tempDir;
    } catch (IOException e) {
        return PackageManager.INSTALL_FAILED_INSUFFICIENT_STORAGE;
    }
    // 在imcs.copyPackage()中会调用target.open()，返回一个文件描述符
    final IParcelFileDescriptorFactory target = new IParcelFileDescriptorFactory.Stub() {
        @Override
        public ParcelFileDescriptor open(String name, int mode) throws RemoteException {
            if (!FileUtils.isValidExtFilename(name)) {
                throw new IllegalArgumentException("Invalid filename: " + name);
            }
            try {
                final File file = new File(codeFile, name);
                final FileDescriptor fd = Os.open(file.getAbsolutePath(),
                        O_RDWR | O_CREAT, 0644);
                Os.chmod(file.getAbsolutePath(), 0644);
                return new ParcelFileDescriptor(fd);
            } catch (ErrnoException e) {
                throw new RemoteException("Failed to open: " + e.getMessage());
            }
        }
    };
    int ret = PackageManager.INSTALL_SUCCEEDED;
    // 调用DefaultContainerService.mBinder.copyPackage()方法复制文件到target.open()方法指定的文件中，也即是上面产生的临时文件
    ret = imcs.copyPackage(origin.file.getAbsolutePath(), target);
    if (ret != PackageManager.INSTALL_SUCCEEDED) {
        return ret;
    }
    final File libraryRoot = new File(codeFile, LIB_DIR_NAME);
    NativeLibraryHelper.Handle handle = null;
    try {
        handle = NativeLibraryHelper.Handle.create(codeFile);
        ret = NativeLibraryHelper.copyNativeBinariesWithOverride(handle, libraryRoot,
                abiOverride);
    } catch (IOException e) {
        ret = PackageManager.INSTALL_FAILED_INTERNAL_ERROR;
    } finally {
        IoUtils.closeQuietly(handle);
    }
    return ret;
}
```
而copyApk方法同样是调用DefaultContainerService的copyPackage将应用的文件复制到/data/app下，如果还有native动态库，也会把包在apk文件中的动态库提取出来。

执行完copyApk后，应用安装到了data/app目录下了。
在pms的构造函数中,定义了相关的路径
```java
File dataDir = Environment.getDataDirectory();
            mAppDataDir = new File(dataDir, "data");
            mAppInstallDir = new File(dataDir, "app");
```
### InstallParams.handleReturnCode()
在handleStartCopy()执行完之后，文件复制工作阶段的工作已经完成了，接下来会在startCopy()中调用handleReturnCode()->processPendingInstall()来进行应用的解析和装载。
## 解析应用阶段
这个阶段的工作是对安装包进行扫描优化，把应用转换成oat格式，然后装载到内存中去。
### processPendingInstall()
```java
private void processPendingInstall(final InstallArgs args, final int currentStatus) {
    // 以异步的方式执行安装，因为安装工作可能持续时间比较长
    mHandler.post(new Runnable() {
        public void run() {
            // 防止重复调用
            mHandler.removeCallbacks(this);
            PackageInstalledInfo res = new PackageInstalledInfo();
            res.returnCode = currentStatus;
            res.uid = -1;
            res.pkg = null;
            res.removedInfo = new PackageRemovedInfo();
            if (res.returnCode == PackageManager.INSTALL_SUCCEEDED) {
                // 如果前面返回的是执行成功的返回值
                args.doPreInstall(res.returnCode);
                synchronized (mInstallLock) {
                    // 开始安装应用，带LI后缀的函数执行时要带mInstallLock锁
                    installPackageLI(args, res);
                }
                // 执行doPostInstall()，这里主要分析一下FileInstallArgs.doPostInstall()
                // 如果没有安装成功，这里会清除前面生成的临时文件
                args.doPostInstall(res.returnCode, res.uid);
            }
            // 执行备份，在下面的情况下会执行备份：1.安装成功，2.是一个新的安装而不是一个升级的操作，3.新的安装包还没有执行过备份操作
            final boolean update = res.removedInfo.removedPackage != null;
            final int flags = (res.pkg == null) ? 0 : res.pkg.applicationInfo.flags;
            boolean doRestore = !update
                    && ((flags & ApplicationInfo.FLAG_ALLOW_BACKUP) != 0);
            // Set up the post-install work request bookkeeping.  This will be used
            // and cleaned up by the post-install event handling regardless of whether
            // there's a restore pass performed.  Token values are >= 1.
            int token;
            if (mNextInstallToken < 0) mNextInstallToken = 1;
            token = mNextInstallToken++;
            PostInstallData data = new PostInstallData(args, res);
            mRunningInstalls.put(token, data);
            if (res.returnCode == PackageManager.INSTALL_SUCCEEDED && doRestore) {
                IBackupManager bm = IBackupManager.Stub.asInterface(
                        ServiceManager.getService(Context.BACKUP_SERVICE));
                if (bm != null) {
                    try {
                        if (bm.isBackupServiceActive(UserHandle.USER_OWNER)) {
                            bm.restoreAtInstall(res.pkg.applicationInfo.packageName, token);
                        } else {
                            doRestore = false;
                        }
                    } catch (RemoteException e) {
                    } catch (Exception e) {
                        doRestore = false;
                    }
                } else {
                    doRestore = false;
                }
            }
            if (!doRestore) {
                // 发送POST_INSTALL消息
                Message msg = mHandler.obtainMessage(POST_INSTALL, token, 0);
                mHandler.sendMessage(msg);
            }
        }
    });
}
```
processPendingInstall()方法内部是以异步的方式继续执行安装工作的，首先来调用installPackageLI()执行安装工作，然后调用doPostInstall()对前面的工作的返回结果进行处理，如果没有安装成功，执行清除的工作。然后再执行备份操作。
下面来看一下installPackageLI()方法：
### installPackageLI()
installPackageLI()方法首先解析apk安装包，然后判断当前是否有安装该应用，然后根据不同的情况进行不同的处理，然后进行Dex优化操作。如果是升级安装，调用replacePackageLI()。如果是新安装，调用installNewPackageLI()。这两个方法会在下面详细介绍。

processPendingInstall()方法中执行安装的最后是发送POST_INSTALL消息，现在来看一下这个消息需要处理的事情：
```java
private void installPackageLI(InstallArgs args, PackageInstalledInfo res) {
    final int installFlags = args.installFlags;
    final String installerPackageName = args.installerPackageName;
    final String volumeUuid = args.volumeUuid;
    final File tmpPackageFile = new File(args.getCodePath());
    final boolean forwardLocked = ((installFlags & PackageManager.INSTALL_FORWARD_LOCK) != 0);
    final boolean onExternal = (((installFlags & PackageManager.INSTALL_EXTERNAL) != 0)
            || (args.volumeUuid != null));
    boolean replace = false;
    int scanFlags = SCAN_NEW_INSTALL | SCAN_UPDATE_SIGNATURE;
    if (args.move != null) {
        scanFlags |= SCAN_INITIAL;
    }
    res.returnCode = PackageManager.INSTALL_SUCCEEDED;
    // 创建apk解析器
    final int parseFlags = mDefParseFlags | PackageParser.PARSE_CHATTY
            | (forwardLocked ? PackageParser.PARSE_FORWARD_LOCK : 0)
            | (onExternal ? PackageParser.PARSE_EXTERNAL_STORAGE : 0);
    PackageParser pp = new PackageParser();
    pp.setSeparateProcesses(mSeparateProcesses);
    pp.setDisplayMetrics(mMetrics);
    final PackageParser.Package pkg;
    try {
        // 开始解析文件，解析apk的信息存储在PackageParser.Package中
        pkg = pp.parsePackage(tmpPackageFile, parseFlags);
    } catch (PackageParserException e) {
        res.setError("Failed parse during installPackageLI", e);
        return;
    }
    ......
    // 获取安装包的签名和AndroidManifest摘要
    try {
        pp.collectCertificates(pkg, parseFlags);
        pp.collectManifestDigest(pkg);
    } catch (PackageParserException e) {
        res.setError("Failed collect during installPackageLI", e);
        return;
    }
    if (args.manifestDigest != null) {
        // 与installPackage()方法传递过来的VerificationParams获取的AndroidManifest摘要进行对比
        // TODO manifestDigest作用？防止menifest穿该？
        if (!args.manifestDigest.equals(pkg.manifestDigest)) {
            res.setError(INSTALL_FAILED_PACKAGE_CHANGED, "Manifest digest changed");
            return;
        }
    } else if (DEBUG_INSTALL) {...}
    // Get rid of all references to package scan path via parser.
    pp = null;
    String oldCodePath = null;
    boolean systemApp = false;
    synchronized (mPackages) {
        // 判断是否是升级当前已有应用
        if ((installFlags & PackageManager.INSTALL_REPLACE_EXISTING) != 0) {
            String oldName = mSettings.mRenamedPackages.get(pkgName);
            if (pkg.mOriginalPackages != null
                    && pkg.mOriginalPackages.contains(oldName)
                    && mPackages.containsKey(oldName)) {
                // 如果当前应用已经被升级过
                pkg.setPackageName(oldName);
                pkgName = pkg.packageName;
                replace = true;
            } else if (mPackages.containsKey(pkgName)) {
                // 当前应用没有被升级过
                replace = true;
            }
            // 如果已有应用oldTargetSdk大于LOLLIPOP_MR1(22)，新升级应用小于LOLLIPOP_MR1，则不允许降级安装
            // 因为AndroidM(23)引入了全新的权限管理方式：动态权限管理
            if (replace) {
                PackageParser.Package oldPackage = mPackages.get(pkgName);
                final int oldTargetSdk = oldPackage.applicationInfo.targetSdkVersion;
                final int newTargetSdk = pkg.applicationInfo.targetSdkVersion;
                if (oldTargetSdk > Build.VERSION_CODES.LOLLIPOP_MR1
                        && newTargetSdk <= Build.VERSION_CODES.LOLLIPOP_MR1) {
                    ...
                    return;
                }
            }
        }
        PackageSetting ps = mSettings.mPackages.get(pkgName);
        if (ps != null) {
            if (shouldCheckUpgradeKeySetLP(ps, scanFlags)) {
                // 判断签名是否一致
                if (!checkUpgradeKeySetLP(ps, pkg)) {
                    ...
                    return;
                }
            } else {
                try {
                    verifySignaturesLP(ps, pkg);
                } catch (PackageManagerException e) {
                    ...
                    return;
                }
            }
            oldCodePath = mSettings.mPackages.get(pkgName).codePathString;
            if (ps.pkg != null && ps.pkg.applicationInfo != null) {
                // 判断是否是系统应用
                systemApp = (ps.pkg.applicationInfo.flags &

            // 给origUsers赋值，此变量代表哪些用户以前已经安装过该应用
            res.origUsers = ps.queryInstalledUsers(sUserManager.getUserIds(), true);
        }
        // Check whether the newly-scanned package wants to define an already-defined perm
        int N = pkg.permissions.size();
        for (int i = N-1; i >= 0; i--) {
            PackageParser.Permission perm = pkg.permissions.get(i);
            BasePermission bp = mSettings.mPermissions.get(perm.info.name);
            if (bp != null) {
                // If the defining package is signed with our cert, it's okay.  This
                // also includes the "updating the same package" case, of course.
                // "updating same package" could also involve key-rotation.
                final boolean sigsOk;
                if (bp.sourcePackage.equals(pkg.packageName)
                        && (bp.packageSetting instanceof PackageSetting)
                        && (shouldCheckUpgradeKeySetLP((PackageSetting) bp.packageSetting,
                                scanFlags))) {
                    sigsOk = checkUpgradeKeySetLP((PackageSetting) bp.packageSetting, pkg);
                } else {
                    sigsOk = compareSignatures(bp.packageSetting.signatures.mSignatures,
                            pkg.mSignatures) == PackageManager.SIGNATURE_MATCH;
                }
                if (!sigsOk) {
                    // If the owning package is the system itself, we log but allow
                    // install to proceed; we fail the install on all other permission
                    // redefinitions.
                    if (!bp.sourcePackage.equals("android")) {
                        res.setError(INSTALL_FAILED_DUPLICATE_PERMISSION, "Package "
                                + pkg.packageName + " attempting to redeclare permission "
                                + perm.info.name + " already owned by " + bp.sourcePackage);
                        res.origPermission = perm.info.name;
                        res.origPackage = bp.sourcePackage;
                        return;
                    } else {
                        pkg.permissions.remove(i);
                    }
                }
            }
        }
    }
    // 系统应用不允许安装在SDCard上
    if (systemApp && onExternal) {
        res.setError(INSTALL_FAILED_INVALID_INSTALL_LOCATION,
                "Cannot install updates to system apps on sdcard");
        return;
    }
    // 下面将会进行Dex优化操作
    if (args.move != null) {
        // 如果是针对已有文件的Move，就不用在进行Dex优化了
        scanFlags |= SCAN_NO_DEX;
        scanFlags |= SCAN_MOVE;
        synchronized (mPackages) {
            final PackageSetting ps = mSettings.mPackages.get(pkgName);
            if (ps == null) {
                res.setError(INSTALL_FAILED_INTERNAL_ERROR,
                        "Missing settings for moved package " + pkgName);
            }
            pkg.applicationInfo.primaryCpuAbi = ps.primaryCpuAbiString;
            pkg.applicationInfo.secondaryCpuAbi = ps.secondaryCpuAbiString;
        }
    } else if (!forwardLocked && !pkg.applicationInfo.isExternalAsec()) {
        // 没有设置了PRIVATE_FLAG_FORWARD_LOCK标志且不是安装在外部SD卡
        // 使能 SCAN_NO_DEX 标志位，在后面的操作中会跳过 dexopt
        scanFlags |= SCAN_NO_DEX;
        try {
            derivePackageAbi(pkg, new File(pkg.codePath), args.abiOverride,
                    true /* extract libs */);
        } catch (PackageManagerException pme) {
            res.setError(INSTALL_FAILED_INTERNAL_ERROR, "Error deriving application ABI");
            return;
        }
        // 进行DexOpt操作，会调用install 的dexopt命令，优化后的文件放在 /data/dalvik-cache/ 下面
        int result = mPackageDexOptimizer
                .performDexOpt(pkg, null /* instruction sets */, false /* forceDex */,
                        false /* defer */, false /* inclDependencies */,
                        true /* boot complete */);
        if (result == PackageDexOptimizer.DEX_OPT_FAILED) {
            res.setError(INSTALL_FAILED_DEXOPT, "Dexopt failed for " + pkg.codePath);
            return;
        }
    }
    // 重命名/data/app/下面应用的目录名字，调用getNextCodePath()来获取目录名称，类似com.android.browser-1
    if (!args.doRename(res.returnCode, pkg, oldCodePath)) {
        res.setError(INSTALL_FAILED_INSUFFICIENT_STORAGE, "Failed rename");
        return;
    }
    startIntentFilterVerifications(args.user.getIdentifier(), replace, pkg);
    if (replace) {
        // 如果是安装升级包，调用replacePackageLI
        replacePackageLI(pkg, parseFlags, scanFlags | SCAN_REPLACING, args.user,
                installerPackageName, volumeUuid, res);
    } else {
        // 如果安装的新应用，调用installNewPackageLI
        installNewPackageLI(pkg, parseFlags, scanFlags | SCAN_DELETE_DATA_ON_FAILURES,
                args.user, installerPackageName, volumeUuid, res);
    }
    synchronized (mPackages) {
        final PackageSetting ps = mSettings.mPackages.get(pkgName);
        if (ps != null) {
            // 安装完成后，给newUsers赋值，此变量代表哪些用户刚刚安装过该应用
            res.newUsers = ps.queryInstalledUsers(sUserManager.getUserIds(), true);
        }
    }
}
```

processPendingInstall()来进行应用的解析和装载

```java
case POST_INSTALL: {
    //从正在安装队列中将当前正在安装的任务删除
    PostInstallData data = mRunningInstalls.get(msg.arg1);
    mRunningInstalls.delete(msg.arg1);
    boolean deleteOld = false;
    if (data != null) {
        InstallArgs args = data.args;
        PackageInstalledInfo res = data.res;
        if (res.returnCode == PackageManager.INSTALL_SUCCEEDED) {
            final String packageName = res.pkg.applicationInfo.packageName;
            res.removedInfo.sendBroadcast(false, true, false);
            Bundle extras = new Bundle(1);
            extras.putInt(Intent.EXTRA_UID, res.uid);
            // 现在已经成功的安装了应用，在发送广播之前先授予一些必要的权限
            // 这些权限在 installPackageAsUser 中创建 InstallParams 时传递的，为null
            if ((args.installFlags
                    & PackageManager.INSTALL_GRANT_RUNTIME_PERMISSIONS) != 0) {
                grantRequestedRuntimePermissions(res.pkg, args.user.getIdentifier(),
                        args.installGrantPermissions);
            }
            // 看一下当前应用对于哪些用户是第一次安装，哪些用户是升级安装
            int[] firstUsers;
            int[] updateUsers = new int[0];
            if (res.origUsers == null || res.origUsers.length == 0) {
                // 所有用户都是第一次安装
                firstUsers = res.newUsers;
            } else {
                firstUsers = new int[0];
                // 这里再从刚刚已经安装该包的用户中选出哪些是以前已经安装过该包的用户
                for (int i=0; i<res.newUsers.length; i++) {
                    int user = res.newUsers[i];
                    boolean isNew = true;
                    for (int j=0; j<res.origUsers.length; j++) {
                        if (res.origUsers[j] == user) {
                            // 找到以前安装过该包的用户
                            isNew = false;
                            break;
                        }
                    }
                    if (isNew) {
                        int[] newFirst = new int[firstUsers.length+1];
                        System.arraycopy(firstUsers, 0, newFirst, 0,
                                firstUsers.length);
                        newFirst[firstUsers.length] = user;
                        firstUsers = newFirst;
                    } else {
                        int[] newUpdate = new int[updateUsers.length+1];
                        System.arraycopy(updateUsers, 0, newUpdate, 0,
                                updateUsers.length);
                        newUpdate[updateUsers.length] = user;
                        updateUsers = newUpdate;
                    }
                }
            }
            //为新安装用户发送广播ACTION_PACKAGE_ADDED
            sendPackageBroadcast(Intent.ACTION_PACKAGE_ADDED,
                    packageName, extras, null, null, firstUsers);
            final boolean update = res.removedInfo.removedPackage != null;
            if (update) {
                extras.putBoolean(Intent.EXTRA_REPLACING, true);
            }
            //为升级安装用户发送广播ACTION_PACKAGE_ADDED
            sendPackageBroadcast(Intent.ACTION_PACKAGE_ADDED,
                    packageName, extras, null, null, updateUsers);
            if (update) {
                // 如果是升级安装，还会发送ACTION_PACKAGE_REPLACED和ACTION_MY_PACKAGE_REPLACED广播
                sendPackageBroadcast(Intent.ACTION_PACKAGE_REPLACED,
                        packageName, extras, null, null, updateUsers);
                sendPackageBroadcast(Intent.ACTION_MY_PACKAGE_REPLACED,
                        null, null, packageName, null, updateUsers);
                // 判断该包是否是设置了PRIVATE_FLAG_FORWARD_LOCK标志或者是安装在外部SD卡
                if (res.pkg.isForwardLocked() || isExternal(res.pkg)) {
                    int[] uidArray = new int[] { res.pkg.applicationInfo.uid };
                    ArrayList<String> pkgList = new ArrayList<String>(1);
                    pkgList.add(packageName);
                    sendResourcesChangedBroadcast(true, true,
                            pkgList,uidArray, null);
                }
            }
            if (res.removedInfo.args != null) {
                // 删除被替换应用的资源目录标记位
                deleteOld = true;
            }
            // 针对Browser的一些处理
            if (firstUsers.length > 0) {
                if (packageIsBrowser(packageName, firstUsers[0])) {
                    synchronized (mPackages) {
                        for (int userId : firstUsers) {
                            mSettings.setDefaultBrowserPackageNameLPw(null, userId);
                        }
                    }
                }
            }
            ...
        }
        // 执行一次GC操作
        Runtime.getRuntime().gc();
        // 执行删除操作
        if (deleteOld) {
            synchronized (mInstallLock) {
                res.removedInfo.args.doPostDeleteLI(true);
            }
        }
        if (args.observer != null) {
            try {
                // 调用回调函数通知安装者此次安装的结果
                Bundle extras = extrasForInstallResult(res);
                args.observer.onPackageInstalled(res.name, res.returnCode,
                        res.returnMsg, extras);
            } catch (RemoteException e) {...}
        }
    } else {...}
} break;
```
installPackageLI()方法首先解析apk安装包，然后判断当前是否有安装该应用，然后根据不同的情况进行不同的处理，然后进行Dex优化操作。如果是升级安装，调用replacePackageLI()。如果是新安装，调用installNewPackageLI()。这两个方法会在下面详细介绍。
processPendingInstall()方法中执行安装的最后是发送POST_INSTALL消息，现在来看一下这个消息需要处理的事情：
###　doHandleMessage-POST_INSTALL

```java
case POST_INSTALL: {
    //从正在安装队列中将当前正在安装的任务删除
    PostInstallData data = mRunningInstalls.get(msg.arg1);
    mRunningInstalls.delete(msg.arg1);
    boolean deleteOld = false;
    if (data != null) {
        InstallArgs args = data.args;
        PackageInstalledInfo res = data.res;
        if (res.returnCode == PackageManager.INSTALL_SUCCEEDED) {
            final String packageName = res.pkg.applicationInfo.packageName;
            res.removedInfo.sendBroadcast(false, true, false);
            Bundle extras = new Bundle(1);
            extras.putInt(Intent.EXTRA_UID, res.uid);
            // 现在已经成功的安装了应用，在发送广播之前先授予一些必要的权限
            // 这些权限在 installPackageAsUser 中创建 InstallParams 时传递的，为null
            if ((args.installFlags
                    & PackageManager.INSTALL_GRANT_RUNTIME_PERMISSIONS) != 0) {
                grantRequestedRuntimePermissions(res.pkg, args.user.getIdentifier(),
                        args.installGrantPermissions);
            }
            // 看一下当前应用对于哪些用户是第一次安装，哪些用户是升级安装
            int[] firstUsers;
            int[] updateUsers = new int[0];
            if (res.origUsers == null || res.origUsers.length == 0) {
                // 所有用户都是第一次安装
                firstUsers = res.newUsers;
            } else {
                firstUsers = new int[0];
                // 这里再从刚刚已经安装该包的用户中选出哪些是以前已经安装过该包的用户
                for (int i=0; i<res.newUsers.length; i++) {
                    int user = res.newUsers[i];
                    boolean isNew = true;
                    for (int j=0; j<res.origUsers.length; j++) {
                        if (res.origUsers[j] == user) {
                            // 找到以前安装过该包的用户
                            isNew = false;
                            break;
                        }
                    }
                    if (isNew) {
                        int[] newFirst = new int[firstUsers.length+1];
                        System.arraycopy(firstUsers, 0, newFirst, 0,
                                firstUsers.length);
                        newFirst[firstUsers.length] = user;
                        firstUsers = newFirst;
                    } else {
                        int[] newUpdate = new int[updateUsers.length+1];
                        System.arraycopy(updateUsers, 0, newUpdate, 0,
                                updateUsers.length);
                        newUpdate[updateUsers.length] = user;
                        updateUsers = newUpdate;
                    }
                }
            }
            //为新安装用户发送广播ACTION_PACKAGE_ADDED
            sendPackageBroadcast(Intent.ACTION_PACKAGE_ADDED,
                    packageName, extras, null, null, firstUsers);
            final boolean update = res.removedInfo.removedPackage != null;
            if (update) {
                extras.putBoolean(Intent.EXTRA_REPLACING, true);
            }
            //为升级安装用户发送广播ACTION_PACKAGE_ADDED
            sendPackageBroadcast(Intent.ACTION_PACKAGE_ADDED,
                    packageName, extras, null, null, updateUsers);
            if (update) {
                // 如果是升级安装，还会发送ACTION_PACKAGE_REPLACED和ACTION_MY_PACKAGE_REPLACED广播
                sendPackageBroadcast(Intent.ACTION_PACKAGE_REPLACED,
                        packageName, extras, null, null, updateUsers);
                sendPackageBroadcast(Intent.ACTION_MY_PACKAGE_REPLACED,
                        null, null, packageName, null, updateUsers);
                // 判断该包是否是设置了PRIVATE_FLAG_FORWARD_LOCK标志或者是安装在外部SD卡
                if (res.pkg.isForwardLocked() || isExternal(res.pkg)) {
                    int[] uidArray = new int[] { res.pkg.applicationInfo.uid };
                    ArrayList<String> pkgList = new ArrayList<String>(1);
                    pkgList.add(packageName);
                    sendResourcesChangedBroadcast(true, true,
                            pkgList,uidArray, null);
                }
            }
            if (res.removedInfo.args != null) {
                // 删除被替换应用的资源目录标记位
                deleteOld = true;
            }
            // 针对Browser的一些处理
            if (firstUsers.length > 0) {
                if (packageIsBrowser(packageName, firstUsers[0])) {
                    synchronized (mPackages) {
                        for (int userId : firstUsers) {
                            mSettings.setDefaultBrowserPackageNameLPw(null, userId);
                        }
                    }
                }
            }
            ...
        }
        // 执行一次GC操作
        // TODO 为啥要执行gc
        Runtime.getRuntime().gc();
        // 执行删除操作
        if (deleteOld) {
            synchronized (mInstallLock) {
                res.removedInfo.args.doPostDeleteLI(true);
            }
        }
        if (args.observer != null) {
            try {
                // 调用回调函数通知安装者此次安装的结果
                Bundle extras = extrasForInstallResult(res);
                args.observer.onPackageInstalled(res.name, res.returnCode,
                        res.returnMsg, extras);
            } catch (RemoteException e) {...}
        }
    } else {...}
} break;

```
对POST_INSTALL消息消息的处理主要就是一些权限处理、发送广播、通知相关应用处理安装结果，然后调用回调函数onPackageInstalled()，这个回调函数是调用installPackage()方法时作为参数传递进来的。
### 总结一下解析应用阶段的工作：
1.	解析apk信息
2.	dexopt操作
3.	更新权限信息
4.	完成安装,发送Intent.ACTION_PACKAGE_ADDED广播

## 1.1.1 其他相关方法分析
### 1.1 getNextCodePath
```java
private File getNextCodePath(File targetDir, String packageName) {
    int suffix = 1;
    File result;
    do {
        result = new File(targetDir, packageName + "-" + suffix);
        suffix++;
    } while (result.exists());
    return result;
}
```
#### replaceSystemPackageLI()
```java
private void replaceSystemPackageLI(PackageParser.Package deletedPackage,
        PackageParser.Package pkg, int parseFlags, int scanFlags, UserHandle user,
        int[] allUsers, boolean[] perUserInstalled, String installerPackageName,
        String volumeUuid, PackageInstalledInfo res) {
    boolean disabledSystem = false;
    boolean updatedSettings = false;
    parseFlags |= PackageParser.PARSE_IS_SYSTEM;
    if ((deletedPackage.applicationInfo.privateFlags&ApplicationInfo.PRIVATE_FLAG_PRIVILEGED)
            != 0) {
        parseFlags |= PackageParser.PARSE_IS_PRIVILEGED;
    }
    String packageName = deletedPackage.packageName;
    if (packageName == null) {
        res.setError(INSTALL_FAILED_REPLACE_COULDNT_DELETE,
                "Attempt to delete null packageName.");
        return;
    }
    PackageParser.Package oldPkg;
    PackageSetting oldPkgSetting;
    // 读取原来应用的信息
    synchronized (mPackages) {
        oldPkg = mPackages.get(packageName);
        oldPkgSetting = mSettings.mPackages.get(packageName);
        if((oldPkg == null) || (oldPkg.applicationInfo == null) ||
                (oldPkgSetting == null)) {
            res.setError(INSTALL_FAILED_REPLACE_COULDNT_DELETE,
                    "Couldn't find package:" + packageName + " information");
            return;
        }
    }
    // 先杀掉原来应用的进程
    killApplication(packageName, oldPkg.applicationInfo.uid, "replace sys pkg");
    res.removedInfo.uid = oldPkg.applicationInfo.uid;
    res.removedInfo.removedPackage = packageName;
    // 删除原有应用包，这个方法后面博客会详细介绍
    removePackageLI(oldPkgSetting, true);
    // writer
    synchronized (mPackages) {
        //把这个结果保存到mSettings中，即在xml文件中用<updated-package>标签标记
        disabledSystem = mSettings.disableSystemPackageLPw(packageName);
        if (!disabledSystem && deletedPackage != null) {
            // 如果包名和资源路径没有变化，分别构造FileInstallArgs和AsecInstallArgs来完成code和resource资源的清除。
            res.removedInfo.args = createInstallArgsForExisting(0,
                    deletedPackage.applicationInfo.getCodePath(),
                    deletedPackage.applicationInfo.getResourcePath(),
                    getAppDexInstructionSets(deletedPackage.applicationInfo));
        } else {
            res.removedInfo.args = null;
        }
    }
    // 调用installd 的 rmcodecache 命令清除代码缓存文件
    deleteCodeCacheDirsLI(pkg.volumeUuid, packageName);
    res.returnCode = PackageManager.INSTALL_SUCCEEDED;
    pkg.applicationInfo.flags |= ApplicationInfo.FLAG_UPDATED_SYSTEM_APP;
    PackageParser.Package newPackage = null;
    try {
        // 开始扫描文件，scanPackageLI在上一篇博客中有详细介绍，这里会解析文件，设置apk路径以及资源路径
        newPackage = scanPackageLI(pkg, parseFlags, scanFlags, 0, user);
        if (newPackage.mExtras != null) {
            // 更新安装时间与升级时间
            final PackageSetting newPkgSetting = (PackageSetting) newPackage.mExtras;
            newPkgSetting.firstInstallTime = oldPkgSetting.firstInstallTime;
            newPkgSetting.lastUpdateTime = System.currentTimeMillis();
            // is the update attempting to change shared user? that isn't going to work...
            if (oldPkgSetting.sharedUser != newPkgSetting.sharedUser) {
                res.setError(INSTALL_FAILED_SHARED_USER_INCOMPATIBLE,
                        "Forbidding shared user change from " + oldPkgSetting.sharedUser
                        + " to " + newPkgSetting.sharedUser);
                updatedSettings = true;
            }
        }
        if (res.returnCode == PackageManager.INSTALL_SUCCEEDED) {
            // 扫描成功，更新配置文件
            updateSettingsLI(newPackage, installerPackageName, volumeUuid, allUsers,
                    perUserInstalled, res, user);
            updatedSettings = true;
        }
    } catch (PackageManagerException e) {
        res.setError("Package couldn't be installed in " + pkg.codePath, e);
    }
    if (res.returnCode != PackageManager.INSTALL_SUCCEEDED) {
        // 如果安装失败，删除新安装的包，恢复以前的应用包
        if (newPackage != null) {
            removeInstalledPackageLI(newPackage, true);
        }
        try {
            scanPackageLI(oldPkg, parseFlags, SCAN_UPDATE_SIGNATURE, 0, user);
        } catch (PackageManagerException e) {...}
        synchronized (mPackages) {
            if (disabledSystem) {
                mSettings.enableSystemPackageLPw(packageName);
            }
            if (updatedSettings) {
                mSettings.setInstallerPackageName(packageName,
                        oldPkgSetting.installerPackageName);
            }
            mSettings.writeLPr();
        }
    }
}
```
### replaceNonSystemPackageLI()
```java
private void replaceNonSystemPackageLI(PackageParser.Package deletedPackage,
        PackageParser.Package pkg, int parseFlags, int scanFlags, UserHandle user,
        int[] allUsers, boolean[] perUserInstalled, String installerPackageName,
        String volumeUuid, PackageInstalledInfo res) {
    String pkgName = deletedPackage.packageName;
    boolean deletedPkg = true;
    boolean updatedSettings = false;
    long origUpdateTime;
    if (pkg.mExtras != null) {
        origUpdateTime = ((PackageSetting)pkg.mExtras).lastUpdateTime;
    } else {
        origUpdateTime = 0;
    }
    // 删除原有的包，这个方法后面博客会详细介绍
    if (!deletePackageLI(pkgName, null, true, null, null, PackageManager.DELETE_KEEP_DATA,
            res.removedInfo, true)) {
        // 删除失败
        res.setError(INSTALL_FAILED_REPLACE_COULDNT_DELETE, "replaceNonSystemPackageLI");
        deletedPkg = false;
    } else {
        // 删除成功
        if (deletedPackage.isForwardLocked() || isExternal(deletedPackage)) {
            // 如果设置PRIVATE_FLAG_FORWARD_LOCK标志或者是安装在外部SD卡，需要发送广播
            final int[] uidArray = new int[] { deletedPackage.applicationInfo.uid };
            final ArrayList<String> pkgList = new ArrayList<String>(1);
            pkgList.add(deletedPackage.applicationInfo.packageName);
            sendResourcesChangedBroadcast(false, true, pkgList, uidArray, null);
        }
        // 调用installd 的 rmcodecache 命令清除代码缓存文件
        deleteCodeCacheDirsLI(pkg.volumeUuid, pkgName);
        try {
            //扫描文件，更新配置文件
            final PackageParser.Package newPackage = scanPackageLI(pkg, parseFlags,
                    scanFlags | SCAN_UPDATE_TIME, System.currentTimeMillis(), user);
            updateSettingsLI(newPackage, installerPackageName, volumeUuid, allUsers,
                    perUserInstalled, res, user);
            updatedSettings = true;
        } catch (PackageManagerException e) {
            res.setError("Package couldn't be installed in " + pkg.codePath, e);
        }
    }
    if (res.returnCode != PackageManager.INSTALL_SUCCEEDED) {
        // 如果安装失败，执行恢复原来的应用的工作
        if(updatedSettings) {
            deletePackageLI(
                    pkgName, null, true, allUsers, perUserInstalled,
                    PackageManager.DELETE_KEEP_DATA,
                            res.removedInfo, true);
        }
        if (deletedPkg) {
            File restoreFile = new File(deletedPackage.codePath);
            boolean oldExternal = isExternal(deletedPackage);
            int oldParseFlags  = mDefParseFlags | PackageParser.PARSE_CHATTY |
                    (deletedPackage.isForwardLocked() ? PackageParser.PARSE_FORWARD_LOCK : 0) |
                    (oldExternal ? PackageParser.PARSE_EXTERNAL_STORAGE : 0);
            int oldScanFlags = SCAN_UPDATE_SIGNATURE | SCAN_UPDATE_TIME;
            try {
                scanPackageLI(restoreFile, oldParseFlags, oldScanFlags, origUpdateTime, null);
            } catch (PackageManagerException e) {
                return;
            }
            synchronized (mPackages) {
                updatePermissionsLPw(deletedPackage.packageName, deletedPackage,
                        UPDATE_PERMISSIONS_ALL);
                mSettings.writeLPr();
            }
        }
    }
}
```
### installNewPackageLI()
```java
private void installNewPackageLI(PackageParser.Package pkg, int parseFlags, int scanFlags,
        UserHandle user, String installerPackageName, String volumeUuid,
        PackageInstalledInfo res) {
    String pkgName = pkg.packageName;
    final boolean dataDirExists = Environment
            .getDataUserPackageDirectory(volumeUuid, UserHandle.USER_OWNER, pkgName).exists();
    synchronized(mPackages) {
        if (mSettings.mRenamedPackages.containsKey(pkgName)) {
            // 和某个应用更改过包名前的名称相同，安装失败
            res.setError(INSTALL_FAILED_ALREADY_EXISTS, "Attempt to re-install " + pkgName
                    + " without first uninstalling package running as "
                    + mSettings.mRenamedPackages.get(pkgName));
            return;
        }
        if (mPackages.containsKey(pkgName)) {
            // 已经有同名的应用，安装失败
            res.setError(INSTALL_FAILED_ALREADY_EXISTS, "Attempt to re-install " + pkgName
                    + " without first uninstalling.");
            return;
        }
    }
    try {
        //扫描文件
        PackageParser.Package newPackage = scanPackageLI(pkg, parseFlags, scanFlags,
                System.currentTimeMillis(), user);
        //更新配置文件
        updateSettingsLI(newPackage, installerPackageName, volumeUuid, null, null, res, user);
        if (res.returnCode != PackageManager.INSTALL_SUCCEEDED) {
            // 如果安装失败，删除已经安装的数据
            deletePackageLI(pkgName, UserHandle.ALL, false, null, null,
                    dataDirExists ? PackageManager.DELETE_KEEP_DATA : 0,
                            res.removedInfo, true);
        }
    } catch (PackageManagerException e) {
        res.setError("Package couldn't be installed in " + pkg.codePath, e);
    }
}

```





## 装载应用

## ref
- http://www.heqiangfly.com/2016/05/12/android-source-code-analysis-package-manager-installation/

- [Android PackageManager相关源码分析之安装应用](http://www.heqiangfly.com/2016/05/12/android-source-code-analysis-package-manager-installation/)
- [PackageManagerService(Android5.1)深入分析（四）安装应用](http://www.aichengxu.com/android/2506357.htm)
- [Android应用程序安装过程解析(源码角度)](http://www.jianshu.com/p/21412a697eb0)
-
http://www.jianshu.com/p/21412a697eb0
http://solart.cc/2016/10/30/install_apk/
一次测试的信息
adb logcat -b system
```test
10-03 17:39:21.892 I/PackageManager(20739): init_copy idx=0: InstallParams{3c107196 file=/data/local/tmp/k.art.debug cid=null}
10-03 17:39:21.896 I/PackageManager(20739): mcs_bound
10-03 17:39:21.896 I/PackageManager(20739): startCopy UserHandle{-1}: InstallParams{3c107196 file=/data/local/tmp/k.art.debug cid=null}
10-03 17:39:21.923 D/PackageManager(20739): installPackageLI: path=/data/app/vmdl828827845.tmp
10-03 17:39:22.027 D/PackageManager(20739): manifestDigest was not present, but parser got: ManifestDigest {mDigest=fe,da,41,e8,49,d6,cd,e5,10,16,26,df,83,1c,24
,cf,eb,1f,7a,fb,be,27,9f,2d,94,92,9c,ce,f2,6d,78,a1,}
10-03 17:39:22.027 W/PackageManager(20739): Package k.art.debug attempting to redeclare system permission android.permission.WRITE_SETTINGS; ignoring new declar
ation
10-03 17:39:22.027 D/PackageManager(20739): Renaming /data/app/vmdl828827845.tmp to /data/app/k.art.debug-1
10-03 17:39:22.028 D/PackageManager(20739): installNewPackageLI: Package{5c4549c k.art.debug}
10-03 17:39:22.043 I/PackageManager(20739): Linking native library dir for /data/app/k.art.debug-1
10-03 17:39:22.043 D/PackageManager(20739): Resolved nativeLibraryRoot for k.art.debug to root=/data/app/k.art.debug-1/lib, isa=true
10-03 17:39:23.427 D/PackageManager(20739): New package installed in /data/app/k.art.debug-1
10-03 17:39:23.467 V/PackageManager(20739): BM finishing package install for 4
10-03 17:39:23.468 D/PackageManager(20739): Sending to user 0: act=android.intent.action.PACKAGE_ADDED dat=package:k.art.debug flg=0x4000000 Bundle[{android.int
ent.extra.UID=10047, android.intent.extra.user_handle=0}]
10-03 17:39:23.468 D/PackageManager(20739): java.lang.RuntimeException: here
10-03 17:39:23.468 D/PackageManager(20739):     at com.android.server.pm.PackageManagerService.sendPackageBroadcast(PackageManagerService.java:8321)
10-03 17:39:23.468 D/PackageManager(20739):     at com.android.server.pm.PackageManagerService$PackageHandler.doHandleMessage(PackageManagerService.java:1066)
10-03 17:39:23.468 D/PackageManager(20739):     at com.android.server.pm.PackageManagerService$PackageHandler.handleMessage(PackageManagerService.java:824)
10-03 17:39:23.468 D/PackageManager(20739):     at android.os.Handler.dispatchMessage(Handler.java:102)
10-03 17:39:23.468 D/PackageManager(20739):     at android.os.Looper.loop(Looper.java:135)
10-03 17:39:23.468 D/PackageManager(20739):     at android.os.HandlerThread.run(HandlerThread.java:61)
10-03 17:39:23.468 D/PackageManager(20739):     at com.android.server.ServiceThread.run(ServiceThread.java:46)
10-03 17:39:31.964 I/PackageManager(20739): mcs_unbind
10-03 17:39:31.965 I/PackageManager(20739): calling disconnectService()
```
[![N|Solid](https://cldup.com/dTxpPi9lDf.thumb.png)](https://nodesource.com/products/nsolid)
 ## MyS-自定义服务
 - http://www.cnblogs.com/welhzh/p/5509125.html
  - http://blog.csdn.net/jinliang_890905/article/details/7320234
 - http://view.inews.qq.com/a/20160930G078QZ00?refer=share_recomnews

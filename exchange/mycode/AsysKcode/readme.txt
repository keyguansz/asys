http://120.77.204.192/blog/2017/02/03/android-data-binding-%E7%B3%BB%E5%88%97%E4%B8%80-%E8%AF%A6%E7%BB%86%E4%BB%8B%E7%BB%8D%E4%B8%8E%E4%BD%BF%E7%94%A8/
并没有想象中的好用，必须一个框架封装来支持会更好。
不好体验：
layout的variable没有编译检查，其实是gradle console才有
方法不能自动补全：android:onClick没有自动补全的功能，MyStringUtils.capitaliz
<?xml version="1.0" encoding="utf-8"?>
<layout xmlns:android="http://schemas.android.com/apk/res/android">
    <data>
        <variable
            name="item"
            type="key.android.demo.databindingdemo.model.RecyclerItem"/>
    </data>

    <LinearLayout
        android:layout_width="match_parent"
        android:layout_height="wrap_content"
        >

        <Button
            android:id="@+id/btn_item"
            android:layout_width="match_parent"
            android:layout_height="wrap_content"        
            android:text="@{item22222.type}"      
           />
    </LinearLayout>

</layout>

重构代码复杂：重命名不能反射到xml里面；
串联字符串不方便：@string/lastname(user.lastName ?

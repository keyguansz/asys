http://120.77.204.192/blog/2017/02/03/android-data-binding-%E7%B3%BB%E5%88%97%E4%B8%80-%E8%AF%A6%E7%BB%86%E4%BB%8B%E7%BB%8D%E4%B8%8E%E4%BD%BF%E7%94%A8/
��û�������еĺ��ã�����һ����ܷ�װ��֧�ֻ���á�
�������飺
layout��variableû�б����飬��ʵ��gradle console����
���������Զ���ȫ��android:onClickû���Զ���ȫ�Ĺ��ܣ�MyStringUtils.capitaliz
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

�ع����븴�ӣ����������ܷ��䵽xml���棻
�����ַ��������㣺@string/lastname(user.lastName ?

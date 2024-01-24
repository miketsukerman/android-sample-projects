package com.example.myapplication

import java.io.InputStream

public class InputStreamNative {
    external fun read(inputStream: InputStream): String
}

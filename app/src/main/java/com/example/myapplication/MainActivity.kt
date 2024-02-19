package com.example.myapplication

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import com.example.myapplication.databinding.ActivityMainBinding
import java.io.InputStream

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    private lateinit var fileReader: FileReader

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        fileReader = FileReader()
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        val inputStream: InputStream = assets.open("example.txt")

        binding.sampleText.text = fileReader.read(inputStream)
    }

    companion object {
        // Used to load the 'myapplication' library on application startup.
        init {
            System.loadLibrary("myapplication")
        }
    }
}
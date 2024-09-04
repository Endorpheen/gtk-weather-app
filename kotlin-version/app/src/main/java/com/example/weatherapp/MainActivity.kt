package com.example.weatherapp

import android.os.Bundle
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.compose.ui.tooling.preview.Preview
import com.example.weatherapp.ui.theme.WeatherAppTheme
import retrofit2.Call
import retrofit2.Callback
import retrofit2.Response
import retrofit2.Retrofit
import retrofit2.converter.gson.GsonConverterFactory
import retrofit2.http.GET
import retrofit2.http.Query

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            WeatherAppTheme {
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = MaterialTheme.colorScheme.background
                ) {
                    WeatherApp()
                }
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun WeatherApp() {
    var city by remember { mutableStateOf("") }
    var weatherInfo by remember { mutableStateOf("Здесь появится информация о погоде") }

    Column(modifier = Modifier.padding(16.dp)) {
        TextField(
            value = city,
            onValueChange = { city = it },
            label = { Text("Введите название города") },
            modifier = Modifier.fillMaxWidth()
        )
        Spacer(modifier = Modifier.height(8.dp))
        Button(onClick = { fetchWeather(city, { weatherInfo = it }, { weatherInfo = "Ошибка: $it" }) }) {
            Text("Узнать погоду")
        }
        Spacer(modifier = Modifier.height(16.dp))
        Text(weatherInfo)
    }
}

fun fetchWeather(city: String, onSuccess: (String) -> Unit, onError: (String) -> Unit) {
    if (city.isEmpty()) {
        onError("Пожалуйста, введите название города")
        return
    }

    val retrofit = Retrofit.Builder()
        .baseUrl("https://api.openweathermap.org/data/2.5/")
        .addConverterFactory(GsonConverterFactory.create())
        .build()

    val weatherService = retrofit.create(WeatherService::class.java)
    val call = weatherService.getWeather(city, "YOUR_API_KEY", "metric")

    call.enqueue(object : Callback<WeatherResponse> {
        override fun onResponse(call: Call<WeatherResponse>, response: Response<WeatherResponse>) {
            if (response.isSuccessful) {
                val weather = response.body()
                weather?.let {
                    val temp = it.main.temp
                    val description = it.weather[0].description
                    onSuccess("Температура: $temp °C\nПогода: $description")
                } ?: run {
                    onError("Ошибка: не удалось получить данные о погоде.")
                }
            } else {
                val errorBody = response.errorBody()?.string()
                Log.e("WeatherApp", "Ошибка при выполнении запроса: ${response.code()}, $errorBody")
                onError("Ошибка: ${response.code()}")
            }
        }

        override fun onFailure(call: Call<WeatherResponse>, t: Throwable) {
            Log.e("WeatherApp", "Ошибка при выполнении запроса: ${t.message}")
            onError("Ошибка: ${t.message}")
        }
    })
}

interface WeatherService {
    @GET("weather")
    fun getWeather(
        @Query("q") city: String,
        @Query("appid") apiKey: String,
        @Query("units") units: String
    ): Call<WeatherResponse>
}

data class WeatherResponse(
    val main: Main,
    val weather: List<Weather>
)

data class Main(
    val temp: Double
)

data class Weather(
    val description: String
)

@Preview(showBackground = true)
@Composable
fun DefaultPreview() {
    WeatherAppTheme {
        WeatherApp()
    }
}
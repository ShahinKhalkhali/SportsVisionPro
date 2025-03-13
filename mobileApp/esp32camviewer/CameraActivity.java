package com.example.esp32camviewer;
import android.os.Bundle;
import android.view.MenuItem;
import android.view.View;
import android.webkit.WebView;
import android.widget.EditText;
import android.widget.Toast;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

public class CameraActivity extends AppCompatActivity {
    private EditText etCameraUrl;
    private WebView webViewStream;
    private boolean isSensorDataRunning;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.camera_activity);

        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        if (getSupportActionBar() != null) {
            getSupportActionBar().setDisplayHomeAsUpEnabled(true);
            getSupportActionBar().setTitle("Camera View");
        }
        initializeViews();
        setupClickListeners();
    }

    private void initializeViews() {
        etCameraUrl = findViewById(R.id.et_camera_url);
        webViewStream = findViewById(R.id.webview_stream);
    }

    private void setupClickListeners() {
        findViewById(R.id.btn_connect).setOnClickListener(v -> {
            String cameraUrl = etCameraUrl.getText().toString().trim();

            if (cameraUrl.isEmpty()) {
                Toast.makeText(CameraActivity.this, "Please enter valid URLs!", Toast.LENGTH_SHORT).show();
                return;
            }
            connectToCamera(cameraUrl);
        });
    }

    private void connectToCamera(String url) {
        if (!url.endsWith("/stream")) {
            url += "/stream";
        }
        webViewStream.setVisibility(View.VISIBLE);
        webViewStream.loadUrl(url);
        Toast.makeText(this, "Connecting to " + url, Toast.LENGTH_SHORT).show();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            finish();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        isSensorDataRunning = false;
    }
}

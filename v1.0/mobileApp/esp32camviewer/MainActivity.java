package com.example.esp32camviewer;
import android.content.Intent;
import android.os.Bundle;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {
    private EditText etUsername, etPassword;
    private Button btnLogin, btnRegister;
    private FirebaseHelper firebaseHelper;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main_activity);

        initializeViews();
        setupClickListeners();

        firebaseHelper = new FirebaseHelper();
    }

    private void initializeViews() {
        etUsername = findViewById(R.id.et_username);
        etPassword = findViewById(R.id.et_password);
        btnLogin = findViewById(R.id.btn_login);
        btnRegister = findViewById(R.id.btn_register);
    }

    private void setupClickListeners() {
        btnLogin.setOnClickListener(v -> {
            String username = etUsername.getText().toString().trim();
            String password = etPassword.getText().toString().trim();

            if (username.isEmpty() || password.isEmpty()) {
                Toast.makeText(MainActivity.this, "Enter username & password", Toast.LENGTH_SHORT).show();
                return;
            }

            firebaseHelper.loginUser(username, password, new FirebaseHelper.FirebaseCallback() {
                @Override
                public void onSuccess(String message) {
                    runOnUiThread(() -> {
                        Toast.makeText(MainActivity.this, message, Toast.LENGTH_SHORT).show();
                        startActivity(new Intent(MainActivity.this, CameraActivity.class));
                    });
                }

                @Override
                public void onError(String error) {
                    runOnUiThread(() -> Toast.makeText(MainActivity.this, error, Toast.LENGTH_SHORT).show());
                }
            });
        });

        btnRegister.setOnClickListener(v -> {
            String username = etUsername.getText().toString().trim();
            String password = etPassword.getText().toString().trim();

            if (username.isEmpty() || password.isEmpty()) {
                Toast.makeText(MainActivity.this, "Enter username & password", Toast.LENGTH_SHORT).show();
                return;
            }
            showTermsDialog(username, password);
        });
    }

    private void showTermsDialog(String username, String password) {
        String termsAndConditions = "By clicking \"Accept\", I acknowledge and agree to these terms:\n\n" +
                "1️⃣ Privacy – My data is stored securely in our database and properly protected.\n" +
                "2️⃣ Fair Usage – Unauthorized access or misuse is prohibited.\n" +
                "3️⃣ Updates – Sports Vision Pro may change these terms at any time.\n\n";

        new AlertDialog.Builder(this)
                .setTitle("Terms & Conditions")
                .setMessage(termsAndConditions)
                .setPositiveButton("Accept", (dialog, which) -> {
                    firebaseHelper.registerUser(username, password, new FirebaseHelper.FirebaseCallback() {
                        @Override
                        public void onSuccess(String message) {
                            runOnUiThread(() -> Toast.makeText(MainActivity.this, "✅ " + message, Toast.LENGTH_SHORT).show());
                        }

                        @Override
                        public void onError(String error) {
                            runOnUiThread(() -> Toast.makeText(MainActivity.this, "❌ " + error, Toast.LENGTH_SHORT).show());
                        }
                    });
                })
                .setNegativeButton("Decline", (dialog, which) -> {
                    Toast.makeText(MainActivity.this, "❌ Registration canceled", Toast.LENGTH_SHORT).show();
                })
                .show();
    }
}

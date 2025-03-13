package com.example.esp32camviewer;
import android.util.Log;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.firestore.FirebaseFirestore;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class FirebaseHelper {
    private static final String TAG = "FirebaseHelper";
    private FirebaseFirestore db;
    private FirebaseAuth auth;
    private ExecutorService executorService;

    public FirebaseHelper() {
        executorService = Executors.newSingleThreadExecutor();
        db = FirebaseFirestore.getInstance();
        auth = FirebaseAuth.getInstance();
    }

    public void registerUser(String username, String password, FirebaseCallback callback) {
        auth.createUserWithEmailAndPassword(username + "@example.com", password)
                .addOnCompleteListener(task -> {
                    if (task.isSuccessful()) {
                        String userId = auth.getCurrentUser().getUid();

                        Map<String, Object> user = new HashMap<>();
                        user.put("username", username);
                        user.put("password", password);

                        db.collection("users").document(userId).set(user)
                                .addOnSuccessListener(aVoid -> {
                                    Log.d(TAG, "✅ User registered successfully!");
                                    callback.onSuccess("User registered successfully!");
                                })
                                .addOnFailureListener(e -> {
                                    Log.e(TAG, "❌ Firestore Error: " + e.getMessage());
                                    callback.onError("Firestore Error: " + e.getMessage());
                                });
                    } else {
                        Log.e(TAG, "❌ Registration failed: " + task.getException().getMessage());
                        callback.onError("Auth Error: " + task.getException().getMessage());
                    }
                });
    }

    public void loginUser(String username, String password, FirebaseCallback callback) {
        auth.signInWithEmailAndPassword(username + "@example.com", password)
                .addOnCompleteListener(task -> {
                    if (task.isSuccessful()) {
                        Log.d(TAG, "✅ Login Successful");
                        callback.onSuccess("Login Successful!");
                    } else {
                        Log.e(TAG, "❌ Login failed: " + task.getException().getMessage());
                        callback.onError("Invalid username or password");
                    }
                });
    }

    public interface FirebaseCallback {
        void onSuccess(String message);
        void onError(String error);
    }
}

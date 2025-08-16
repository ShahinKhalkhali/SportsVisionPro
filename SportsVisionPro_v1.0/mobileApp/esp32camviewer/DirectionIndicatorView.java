package com.example.esp32camviewer;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.util.AttributeSet;
import android.view.View;

public class DirectionIndicatorView extends View {
    private Paint dotPaint;
    private Paint trianglePaint;
    private Path trianglePath;
    private float rotation = 0f;

    public DirectionIndicatorView(Context context) {
        super(context);
        init();
    }

    public DirectionIndicatorView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    private void init() {
        dotPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        dotPaint.setColor(Color.BLUE);
        dotPaint.setStyle(Paint.Style.FILL);

        trianglePaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        trianglePaint.setColor(Color.argb(50, 0, 0, 255));
        trianglePaint.setStyle(Paint.Style.FILL);

        trianglePath = new Path();
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
        float centerX = w / 2f;
        float centerY = h / 2f;

        trianglePath.reset();
        float triangleHeight = h / 3f;
        float triangleBase = w / 4f;

        trianglePath.moveTo(centerX, centerY - triangleHeight);
        trianglePath.lineTo(centerX - triangleBase, centerY + triangleHeight);
        trianglePath.lineTo(centerX + triangleBase, centerY + triangleHeight);
        trianglePath.close();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        int centerX = getWidth() / 2;
        int centerY = getHeight() / 2;

        canvas.save();
        canvas.rotate(rotation, centerX, centerY);
        canvas.drawPath(trianglePath, trianglePaint);
        canvas.restore();

        float dotRadius = getWidth() / 20f;
        canvas.drawCircle(centerX, centerY, dotRadius, dotPaint);
    }

    public void setRotation(float degrees) {
        this.rotation = degrees;
        invalidate();
    }
}

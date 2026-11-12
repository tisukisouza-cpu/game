package com.ravena.engine;

import android.app.Activity;
import android.os.Bundle;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Color;
import android.view.Gravity;
import android.view.View;
import android.widget.*;
import android.graphics.Typeface;

public class MainActivity extends Activity {

    int bg = Color.rgb(20, 22, 28);
    int panel = Color.rgb(29, 32, 40);
    int accent = Color.rgb(145, 65, 220);
    int white = Color.rgb(230, 232, 240);
    int muted = Color.rgb(145, 150, 165);

    LinearLayout root;

    TextView text(String value, int size, int color) {
        TextView t = new TextView(this);
        t.setText(value);
        t.setTextSize(size);
        t.setTextColor(color);
        t.setPadding(12, 10, 12, 10);
        return t;
    }

    LinearLayout column() {
        LinearLayout l = new LinearLayout(this);
        l.setOrientation(LinearLayout.VERTICAL);
        l.setBackgroundColor(panel);
        return l;
    }

    LinearLayout row() {
        LinearLayout l = new LinearLayout(this);
        l.setOrientation(LinearLayout.HORIZONTAL);
        return l;
    }

    void addPanel(LinearLayout parent, View child, int width) {
        parent.addView(child, new LinearLayout.LayoutParams(
            width, LinearLayout.LayoutParams.MATCH_PARENT
        ));
    }

    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);

        getWindow().setStatusBarColor(bg);
        getWindow().setNavigationBarColor(bg);

        root = column();
        root.setBackgroundColor(bg);

        // Barra superior
        LinearLayout toolbar = row();
        toolbar.setGravity(Gravity.CENTER_VERTICAL);
        toolbar.setBackgroundColor(Color.rgb(25, 27, 34));

        TextView logo = text("RAVENA", 19, white);
        logo.setTypeface(null, Typeface.BOLD);
        toolbar.addView(logo);

        TextView version = text("ENGINE  |  EDITOR 1.0", 10, muted);
        toolbar.addView(version);

        TextView play = text("▶  EXECUTAR", 13, Color.WHITE);
        play.setBackgroundColor(accent);
        toolbar.addView(play);

        root.addView(toolbar, new LinearLayout.LayoutParams(
            -1, 52
        ));

        // Área principal
        LinearLayout workspace = row();

        // Hierarquia
        LinearLayout hierarchy = column();
        hierarchy.addView(text("HIERARQUIA", 12, white));
        hierarchy.addView(text("▾  CENA PRINCIPAL", 12, muted));
        hierarchy.addView(text("   ◈ Câmera", 12, white));
        hierarchy.addView(text("   ◇ Luz Direcional", 12, white));
        hierarchy.addView(text("   ▣ Cubo", 12, accent));
        addPanel(workspace, hierarchy, 155);

        // Viewport 3D
        LinearLayout center = column();
        center.addView(text("VISUALIZAÇÃO 3D     |     PERSPECTIVA", 11, muted));

        View viewport = new Viewport();
        center.addView(viewport, new LinearLayout.LayoutParams(
            -1, 0, 1
        ));

        // Navegador de assets
        LinearLayout assets = column();
        assets.addView(text("NAVEGADOR DE ASSETS", 11, white));
        assets.addView(text("📁 Models     📁 Textures     📁 Materials", 11, muted));
        center.addView(assets, new LinearLayout.LayoutParams(
            -1, 75
        ));

        workspace.addView(center, new LinearLayout.LayoutParams(
            0, -1, 1
        ));

        // Inspetor
        LinearLayout inspector = column();
        inspector.addView(text("INSPETOR", 12, white));
        inspector.addView(text("Cubo", 17, accent));
        inspector.addView(text("TRANSFORMAÇÃO", 11, muted));
        inspector.addView(text("Posição\\nX: 0.0\\nY: 0.0\\nZ: 0.0", 12, white));
        inspector.addView(text("ROTAÇÃO\\nX: 0°\\nY: 0°\\nZ: 0°", 12, white));
        inspector.addView(text("ESCALA\\nX: 1.0\\nY: 1.0\\nZ: 1.0", 12, white));
        addPanel(workspace, inspector, 175);

        root.addView(workspace, new LinearLayout.LayoutParams(
            -1, 0, 1
        ));

        setContentView(root);
    }

    class Viewport extends View {

        Paint p = new Paint(Paint.ANTI_ALIAS_FLAG);

        Viewport() {
            super(MainActivity.this);
        }

        @Override
        protected void onDraw(Canvas c) {
            super.onDraw(c);

            float w = getWidth();
            float h = getHeight();

            p.setColor(Color.rgb(24, 29, 39));
            c.drawColor(p.getColor());

            // Grade do chão
            p.setColor(Color.rgb(53, 61, 76));
            p.setStrokeWidth(1);

            for (int i = 0; i < 20; i++) {
                float x = i * w / 19;
                c.drawLine(x, h * 0.48f, w / 2, h, p);
            }

            for (int i = 0; i < 12; i++) {
                float y = h * 0.48f + i * h * 0.045f;
                c.drawLine(0, y, w, y, p);
            }

            // Cubo demonstrativo
            float cx = w / 2;
            float cy = h / 2;

            p.setColor(Color.rgb(145, 65, 220));
            p.setStrokeWidth(3);
            p.setStyle(Paint.Style.STROKE);

            c.drawRect(cx - 45, cy - 45, cx + 45, cy + 45, p);
            c.drawRect(cx - 20, cy - 70, cx + 70, cy + 20, p);

            c.drawLine(cx - 45, cy - 45, cx - 20, cy - 70, p);
            c.drawLine(cx + 45, cy - 45, cx + 70, cy - 70, p);
            c.drawLine(cx - 45, cy + 45, cx - 20, cy + 20, p);
            c.drawLine(cx + 45, cy + 45, cx + 70, cy + 20, p);

            p.setStyle(Paint.Style.FILL);
            p.setColor(Color.WHITE);
            p.setTextSize(12);
            c.drawText("RAVENA 3D VIEWPORT", 15, 25, p);
        }
    }
}

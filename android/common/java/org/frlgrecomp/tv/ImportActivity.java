package org.frlgrecomp.tv;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.provider.OpenableColumns;

import org.gbarecomp.GbaGameConfig;
import org.gbarecomp.GbaSetupActivity;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.security.MessageDigest;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

/**
 * "Open with" / "Share" import for devices whose file manager is a separate
 * app — typical on Google TV, where Android's system file picker (used by the
 * setup screen's CHOOSE buttons) is often missing.
 *
 * Pick the ROM and/or gba_bios.bin in any file manager, choose Open with or
 * Share, then this game. Each file is hash-verified exactly like the setup
 * screen does (size + SHA-1 against the manifest's accepted dumps) and copied
 * into the same app-private paths, so the setup screen sees it as ready.
 * Nothing is ever bundled in the APK.
 */
public final class ImportActivity extends Activity {
    private static final long BIOS_SIZE = 16 * 1024;

    private GbaGameConfig config;
    private AlertDialog dialog;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        config = GbaGameConfig.load(this);
        final List<Uri> uris = incomingUris(getIntent());
        if (uris.isEmpty()) {
            openSetup();
            return;
        }
        dialog = new AlertDialog.Builder(this)
            .setTitle(config.title)
            .setMessage("Checking file…")
            .setCancelable(false)
            .show();
        new Thread(() -> {
            final StringBuilder report = new StringBuilder();
            boolean anyAccepted = false;
            for (Uri uri : uris) {
                try {
                    report.append(install(uri)).append("\n\n");
                    anyAccepted = true;
                } catch (Exception error) {
                    report.append(error.getMessage()).append("\n\n");
                }
            }
            final boolean accepted = anyAccepted;
            runOnUiThread(() -> showResult(accepted, report.toString().trim()));
        }, "frlg-import").start();
    }

    private void showResult(boolean accepted, String message) {
        if (isFinishing()) return;
        if (dialog != null) dialog.dismiss();
        boolean romReady = romFile().length() == config.romSize;
        boolean biosReady = !config.biosRequired || biosFile().length() == BIOS_SIZE;
        String next = romReady && biosReady ? "\n\nEverything is ready."
            : !romReady ? "\n\nStill needed: the " + config.romLabel + "."
            : "\n\nStill needed: the GBA BIOS (gba_bios.bin).";
        AlertDialog.Builder builder = new AlertDialog.Builder(this)
            .setTitle(accepted ? "Imported" : "Not accepted")
            .setMessage(message + next)
            .setOnCancelListener(d -> finish());
        if (romReady && biosReady) {
            builder.setPositiveButton("PLAY", (d, w) -> openSetup())
                   .setNegativeButton("LATER", (d, w) -> finish());
        } else {
            builder.setPositiveButton("OK", (d, w) -> finish());
        }
        dialog = builder.show();
    }

    private void openSetup() {
        Intent intent = new Intent(this, GbaSetupActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP);
        startActivity(intent);
        finish();
    }

    @SuppressWarnings("deprecation")
    private static List<Uri> incomingUris(Intent intent) {
        List<Uri> uris = new ArrayList<>();
        if (intent == null) return uris;
        String action = intent.getAction();
        if (Intent.ACTION_VIEW.equals(action) && intent.getData() != null) {
            uris.add(intent.getData());
        } else if (Intent.ACTION_SEND.equals(action)) {
            Uri uri = intent.getParcelableExtra(Intent.EXTRA_STREAM);
            if (uri != null) uris.add(uri);
        } else if (Intent.ACTION_SEND_MULTIPLE.equals(action)) {
            ArrayList<Uri> list = intent.getParcelableArrayListExtra(Intent.EXTRA_STREAM);
            if (list != null) uris.addAll(list);
        }
        return uris;
    }

    private File biosFile() { return new File(getFilesDir(), "bios/gba_bios.bin"); }
    private File romFile() { return new File(getFilesDir(), "roms/" + config.romFile); }

    /** Verifies one file, installs it as ROM or BIOS, and returns a summary. */
    private String install(Uri uri) throws Exception {
        String name = displayName(uri);
        long limit = Math.max(config.romSize, BIOS_SIZE);
        File staging = new File(getFilesDir(), "import");
        if (!staging.isDirectory() && !staging.mkdirs())
            throw new IOException("Could not create the private import directory.");
        File temporary = File.createTempFile("import-", ".tmp", staging);
        MessageDigest digest = MessageDigest.getInstance("SHA-1");
        long total = 0;
        try (InputStream input = getContentResolver().openInputStream(uri);
             FileOutputStream output = new FileOutputStream(temporary)) {
            if (input == null) throw new IOException("Android could not open " + name + ".");
            byte[] buffer = new byte[1024 * 1024];
            int read;
            while ((read = input.read(buffer)) >= 0) {
                if (read == 0) continue;
                total += read;
                if (total > limit) throw new IOException(name + " is too large to be a "
                    + config.title + " ROM or GBA BIOS.");
                digest.update(buffer, 0, read);
                output.write(buffer, 0, read);
            }
            output.getFD().sync();
        } catch (IOException error) {
            temporary.delete();
            throw error;
        }
        String sha1 = hex(digest.digest());
        File destination;
        String kind;
        if (total == BIOS_SIZE && config.biosSha1.equals(sha1)) {
            destination = biosFile();
            kind = "GBA BIOS";
        } else if (total == config.romSize && config.romAccepted(sha1)) {
            destination = romFile();
            kind = config.romLabel;
        } else {
            temporary.delete();
            throw new IOException(String.format(Locale.US,
                "%s is not the supported %s or GBA BIOS.\nSize: %,d bytes\nSHA-1: %s",
                name, config.romLabel, total, sha1));
        }
        File directory = destination.getParentFile();
        if (directory != null && !directory.isDirectory() && !directory.mkdirs()) {
            temporary.delete();
            throw new IOException("Could not create the private game directory.");
        }
        if (destination.exists() && !destination.delete()) {
            temporary.delete();
            throw new IOException("Could not replace the previously imported file.");
        }
        if (!temporary.renameTo(destination)) {
            temporary.delete();
            throw new IOException("Could not store " + name + ".");
        }
        return "✓ " + kind + " verified (" + name + ").";
    }

    private String displayName(Uri uri) {
        try (Cursor cursor = getContentResolver().query(
                uri, new String[] {OpenableColumns.DISPLAY_NAME}, null, null, null)) {
            if (cursor != null && cursor.moveToFirst()) {
                int column = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
                if (column >= 0 && cursor.getString(column) != null) return cursor.getString(column);
            }
        } catch (Exception ignored) {
        }
        String last = uri.getLastPathSegment();
        return last != null ? last : "the selected file";
    }

    private static String hex(byte[] bytes) {
        StringBuilder out = new StringBuilder(bytes.length * 2);
        for (byte b : bytes) out.append(String.format(Locale.US, "%02x", b & 0xff));
        return out.toString();
    }
}

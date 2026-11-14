# Resolve the current user's Saved Games known folder (including redirection).
# Used only by Makefile.win at build time, never by the Rogue executable.
# Emits ONE UTF-8 path with forward slashes, no BOM, followed by a newline.
param([switch]$Create)
$ErrorActionPreference = 'Stop'
try {
    Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;

public static class RogueWindowsKnownFolder {
    [DllImport("ole32.dll")]
    private static extern int CoInitializeEx(IntPtr pvReserved, uint dwCoInit);
    [DllImport("ole32.dll")]
    private static extern void CoUninitialize();
    [DllImport("ole32.dll")]
    private static extern void CoTaskMemFree(IntPtr pv);
    [DllImport("shell32.dll", ExactSpelling = true)]
    private static extern int SHGetKnownFolderPath(
        ref Guid rfid, uint dwFlags, IntPtr hToken, out IntPtr ppszPath);

    public static string GetSavedGamesPath() {
        // FOLDERID_SavedGames: https://learn.microsoft.com/windows/win32/shell/knownfolderid
        Guid id = new Guid("4C5C32FF-BB9D-43B0-B5B4-2D72E54EAAA4");
        // COINIT_APARTMENTTHREADED; tolerate an already initialized MTA.
        int init = CoInitializeEx(IntPtr.Zero, 2);
        if (init < 0 && init != unchecked((int)0x80010106))
            Marshal.ThrowExceptionForHR(init);
        IntPtr path = IntPtr.Zero;
        try {
            // KF_FLAG_DONT_VERIFY: permit resolving a directory that is not yet present.
            int hr = SHGetKnownFolderPath(ref id, 0x00004000, IntPtr.Zero, out path);
            Marshal.ThrowExceptionForHR(hr);
            if (path == IntPtr.Zero)
                throw new InvalidOperationException("Saved Games has no path.");
            return Marshal.PtrToStringUni(path);
        }
        finally {
            if (path != IntPtr.Zero) CoTaskMemFree(path);
            if (init >= 0) CoUninitialize();
        }
    }
}
'@
    $savedGames = [RogueWindowsKnownFolder]::GetSavedGamesPath()
    if ([string]::IsNullOrWhiteSpace($savedGames) -or
        -not [System.IO.Path]::IsPathRooted($savedGames)) {
        throw 'Windows returned an empty or non-absolute Saved Games path.'
    }
    $result = [System.IO.Path]::Combine($savedGames, 'Rogue Clone III').Replace('\', '/')
    if ($Create) {
        # Create through Windows Unicode APIs; do not round-trip via shell quoting.
        [System.IO.Directory]::CreateDirectory($result) | Out-Null
        exit 0
    }
    # PowerShell 5.1 encodes redirected native output with its console code page.
    # Write bytes directly so MSYS2 GNU make receives UTF-8 even for Japanese paths.
    $bytes = ([System.Text.UTF8Encoding]::new($false)).GetBytes($result + "`n")
    $stdout = [Console]::OpenStandardOutput()
    $stdout.Write($bytes, 0, $bytes.Length)
    $stdout.Flush()
}
catch {
    [Console]::Error.WriteLine('Cannot resolve Windows Saved Games: ' + $_.Exception.Message)
    exit 1
}

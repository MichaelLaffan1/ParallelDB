$outputFile = "performance_log.csv"
$exePath = "ParallelDatabase.exe"

# Write CSV header
"FileName,NumOfNodes,RuntimeSeconds,PeakMemoryMB" | Out-File -FilePath $outputFile

Get-ChildItem -Path . -Filter "sql_*_insertBias70.sql" | ForEach-Object {
#Get-ChildItem -Path . -Filter "sql_100000_insertBias70.sql" | ForEach-Object {
    $inputFile = $_.FullName
    $baseName = $_.BaseName

    # Run the program with different numbers of nodes
    For ($numOfNodes = 1; $numOfNodes -le 12; $numOfNodes++) {
        $tupleCountFile = "$($baseName)_tuple_counts.csv"

        # Measure runtime
        $runtime = Measure-Command {
            Start-Process -FilePath "mpiexec" `
                -ArgumentList "-n $numOfNodes $exePath -i $inputFile -t $tupleCountFile" `
                -NoNewWindow -Wait
        }

        # Capture peak memory usage
        $process = Start-Process -FilePath "mpiexec" `
            -ArgumentList "-n $numOfNodes $exePath -i $inputFile -t $tupleCountFile" `
            -NoNewWindow -PassThru
        Start-Sleep -Milliseconds 100  # Allow process initialization

        try {
            $peakMemory = Get-Process -Id $process.Id -ErrorAction Stop |
                          Select-Object -ExpandProperty PeakWorkingSet64
            $peakMemoryMB = [math]::Round($peakMemory / 1MB, 2)
        } catch {
            $peakMemoryMB = "N/A"  # If the process ends too quickly, log as "N/A"
        }

        "$baseName,$numOfNodes,$($runtime.TotalSeconds),$peakMemoryMB" | Out-File -FilePath $outputFile -Append
    }
}

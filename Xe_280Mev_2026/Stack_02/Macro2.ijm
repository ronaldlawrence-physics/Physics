// ================================================
// ImageJ Macro - Emulsion Grain Analysis (with Skip)
// Skips plates that already have Results + Summary files
// ================================================

inputDir = "/home/ronald/JINR/LHEP/Data/Xe_280Mev_2026/Stack_02/Emulsion_Files";
baseDir = "/home/ronald/JINR/LHEP/Data/Xe_280Mev_2026/Stack_02";
resultsDir = baseDir + "/Results_Files";
summaryDir = baseDir + "/Summary_Files";

// Create output directories if missing
if (!File.exists(resultsDir)) File.makeDirectory(resultsDir);
if (!File.exists(summaryDir)) File.makeDirectory(summaryDir);

combinedSummary = summaryDir + "/All_Plates_Summary.csv";
if (File.exists(combinedSummary)) File.delete(combinedSummary);

header = "Image,Count,Total_Area_um2,Avg_Area_um2,Min_Area_um2,Max_Area_um2,StdDev_um2," +
         "Avg_Circularity,Avg_Roundness,Avg_Solidity,Avg_Feret_um,Avg_Perimeter_um\n";
File.append(header, combinedSummary);

list = getFileList(inputDir);
count = 0;
processed = 0;
firstImageProcessed = false;

for (i = 0; i < list.length; i++) {
    file = list[i];
    fileLower = toLowerCase(file);
    
    // Check if it's a plate image
    if ((startsWith(file, "1_Plate") || startsWith(file, "2_Plate") || startsWith(file, "3_Plate") ||
         startsWith(file, "4_Plate") || startsWith(file, "5_Plate") || startsWith(file, "6_Plate") ||
         startsWith(file, "7_Plate") || startsWith(file, "8_Plate") || startsWith(file, "9_Plate") ||
         startsWith(file, "10_Plate") || indexOf(file, "_Plate_") > 0) &&
        (endsWith(fileLower, ".tif") || endsWith(fileLower, ".tiff"))) {
        
        count++;
        baseName = replace(file, ".tif", "");
        baseName = replace(baseName, ".tiff", "");
        
        // === SKIP CHECK ===
        resultsFile = resultsDir + "/" + baseName + "_Results.csv";
        summaryFile = summaryDir + "/" + baseName + "_Summary.csv";
        
        if (File.exists(resultsFile) && File.exists(summaryFile)) {
            print("[" + count + "] SKIPPED (already processed): " + file);
            continue;
        }
        
        print("[" + count + "] Processing: " + file);
        processed++;
        
        open(inputDir + "/" + file);
        imageTitle = getTitle();
        
        if (!firstImageProcessed) {
            run("Set Scale...", "distance=1.717 known=1 pixel=1 unit=um");
            run("Set Measurements...", "area mean perimeter feret center integrated circularity shape display redirect=None decimal=3");
            firstImageProcessed = true;
        }
        
        // Image Processing
        run("Enhance Contrast", "saturated=0.35 normalize");
        setThreshold(0, 75);
        run("Convert to Mask");
        
        run("Analyze Particles...", "size=10-1200 circularity=0.00-1.00 show=Nothing display exclude include add");
        
        if (nResults > 0) {
            saveAs("Results", resultsFile);
            
            // Summary calculations
            n = nResults;
            totalArea = 0; sumCirc = 0; sumRound = 0; sumSolid = 0;
            sumFeret = 0; sumPeri = 0;
            minArea = getResult("Area", 0);
            maxArea = getResult("Area", 0);
            
            for (j = 0; j < n; j++) {
                a = getResult("Area", j);
                totalArea += a;
                sumCirc += getResult("Circularity", j);
                sumRound += getResult("Round", j);
                sumSolid += getResult("Solidity", j);
                sumFeret += getResult("Feret", j);
                sumPeri += getResult("Perimeter", j);
                
                if (a < minArea) minArea = a;
                if (a > maxArea) maxArea = a;
            }
            
            avgArea = totalArea / n;
            stdDev = 0;
            for (j = 0; j < n; j++) {
                stdDev += pow(getResult("Area", j) - avgArea, 2);
            }
            stdDev = sqrt(stdDev / n);
            
            summaryLine = baseName + "," + n + "," + totalArea + "," + avgArea + "," + minArea + "," + 
                         maxArea + "," + stdDev + "," + (sumCirc/n) + "," + (sumRound/n) + "," + 
                         (sumSolid/n) + "," + (sumFeret/n) + "," + (sumPeri/n) + "\n";
            
            // Save files
            File.append(summaryLine, combinedSummary);
            
            individualSummary = summaryDir + "/" + baseName + "_Summary.csv";
            File.append(header, individualSummary);
            File.append(summaryLine, individualSummary);
            
            print("   → Saved Results & Summary");
        } else {
            print("   → No particles found");
        }
        
        close(imageTitle);
        if (isOpen("Results")) close("Results");
    }
}

print("\n=== PROCESSING COMPLETE ===");
print("Total plates found : " + count);
print("Skipped            : " + (count - processed));
print("Newly processed    : " + processed);
print("Results  → " + resultsDir);
print("Summaries → " + summaryDir);

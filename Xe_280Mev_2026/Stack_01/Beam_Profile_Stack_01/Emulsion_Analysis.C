// ========================================================
// Emulsion_Analysis.C - Full Analysis (All 8 Plates)
// Reads: 5.Results_Files/ and 6.Summary_Files/
// Saves: 7.Plots/ with plate subfolders
// Run with: root -l Emulsion_Analysis.C
// ========================================================

#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TTree.h>
#include <TFile.h>
#include <TPaveText.h>
#include <TLegend.h>
#include <TStyle.h>
#include <TString.h>
#include <TObjArray.h>
#include <TObjString.h>
#include <TMath.h>
#include <TGaxis.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>

void ProcessPlate(TString platePrefix, TString resultsFile, TString summaryFile, TString outDir);

void Emulsion_Analysis()
{
    printf("\n===========================================================\n");
    printf("         EMULSION GRAIN ANALYSIS - ALL PLATES\n");
    printf("===========================================================\n");
    
    // Define all 8 plates in correct order
    TString plateNames[8] = {
        "1_Plate_09",
        "2_Plate_08",
        "3_Plate_07",
        "4_Plate_06",
        "5_Plate_05",
        "6_Plate_04",
        "7_Plate_03",
        "8_Plate_02"
    };
    
    // Create main output directory
    gSystem->mkdir("7.Plots", kTRUE);
    
    // Process each plate
    for (int i = 0; i < 8; i++) {
        TString platePrefix = plateNames[i];
        
        // Input file paths
        TString resultsFile = "5.Results_Files/" + platePrefix + "_Results.csv";
        TString summaryFile = "6.Summary_Files/" + platePrefix + "_Summary.csv";
        TString outDir = "7.Plots/" + platePrefix;
        
        printf("\n===========================================================\n");
        printf("Processing: %s\n", platePrefix.Data());
        printf("===========================================================\n");
        
        // Check if files exist
        if (gSystem->AccessPathName(resultsFile)) {
            printf("WARNING: Results file not found: %s\n", resultsFile.Data());
            printf("Skipping %s\n", platePrefix.Data());
            continue;
        }
        
        if (gSystem->AccessPathName(summaryFile)) {
            printf("WARNING: Summary file not found: %s\n", summaryFile.Data());
            printf("Skipping %s\n", platePrefix.Data());
            continue;
        }
        
        // Process this plate
        ProcessPlate(platePrefix, resultsFile, summaryFile, outDir);
    }
    
    printf("\n===========================================================\n");
    printf("         ANALYSIS COMPLETE FOR ALL PLATES\n");
    printf("===========================================================\n");
    printf("All plots saved in: 7.Plots/\n");
    printf("===========================================================\n");
}

// ========================================================
// Process Single Plate
// ========================================================
void ProcessPlate(TString platePrefix, TString resultsFile, TString summaryFile, TString outDir)
{
    // Calibration factor (µm per pixel)
    const Double_t k = 1.717;
    
    // ====================== READ SUMMARY FILE ======================
    printf("\n[1] Reading summary file: %s\n", summaryFile.Data());
    
    FILE *fSum = fopen(summaryFile, "r");
    if (!fSum) {
        printf("ERROR: Cannot open summary file\n");
        return;
    }
    
    // Skip header
    char header[4096];
    if (!fgets(header, sizeof(header), fSum)) {
        printf("ERROR: Summary file is empty or has no header\n");
        fclose(fSum);
        return;
    }
    printf("   Summary header: %s", header);
    
    // Read summary data
    char line[4096];
    if (!fgets(line, sizeof(line), fSum)) {
        printf("ERROR: Summary file has no data line\n");
        fclose(fSum);
        return;
    }
    fclose(fSum);
    
    // Parse summary line
    TString sLine(line);
    sLine.ReplaceAll("\n", "");
    sLine.ReplaceAll("\r", "");
    printf("   Summary data line: %s\n", sLine.Data());
    
    TObjArray *sumTok = sLine.Tokenize(",");
    sumTok->SetOwner(kTRUE);
    int nSumTokens = sumTok->GetEntries();
    printf("   Summary has %d tokens\n", nSumTokens);
    
    if (nSumTokens < 15) {
        printf("ERROR: Summary file has insufficient columns (%d, need >=15)\n", nSumTokens);
        delete sumTok;
        return;
    }
    
    // Extract summary values with safety checks
    TString sliceName = ((TObjString*)sumTok->At(0))->String();
    Double_t summaryCount = ((TObjString*)sumTok->At(1))->String().Atof();
    Double_t summaryTotalArea = ((TObjString*)sumTok->At(2))->String().Atof();
    Double_t summaryAvgSize = ((TObjString*)sumTok->At(3))->String().Atof();
    Double_t summaryPercentArea = ((TObjString*)sumTok->At(4))->String().Atof();
    Double_t summaryMean = ((TObjString*)sumTok->At(5))->String().Atof();
    Double_t summaryPerim = ((TObjString*)sumTok->At(6))->String().Atof();
    Double_t summaryCirc = ((TObjString*)sumTok->At(7))->String().Atof();
    Double_t summarySolidity = ((TObjString*)sumTok->At(8))->String().Atof();
    Double_t summaryFeret = ((TObjString*)sumTok->At(9))->String().Atof();
    Double_t summaryFeretX = ((TObjString*)sumTok->At(10))->String().Atof();
    Double_t summaryFeretY = ((TObjString*)sumTok->At(11))->String().Atof();
    Double_t summaryFeretAngle = ((TObjString*)sumTok->At(12))->String().Atof();
    Double_t summaryMinFeret = ((TObjString*)sumTok->At(13))->String().Atof();
    Double_t summaryIntDen = ((TObjString*)sumTok->At(14))->String().Atof();
    
    delete sumTok;
    
    printf("   Summary count: %.0f grains\n", summaryCount);
    
    // Derive plate dimensions from summary data
    // Plate area = Total grain area / (%Area / 100)
    Double_t plateArea_um2 = summaryTotalArea / (summaryPercentArea / 100.0);
    Double_t plateArea_mm2 = plateArea_um2 / 1e6;
    // Approximate as square for FOV stats (actual extent computed later from data)
    Double_t plateSize_um = sqrt(plateArea_um2);
    Double_t fovW_mm = plateSize_um / 1000.0;
    Double_t fovH_mm = fovW_mm;
    Double_t fovArea_mm2 = plateArea_mm2;
    
    printf("   Plate area: %.0f um2 (%.2f mm2)\n", plateArea_um2, plateArea_mm2);
    
    // ====================== READ RESULTS FILE ======================
    printf("\n[2] Reading grain data from: %s\n", resultsFile.Data());
    
    FILE *fRes = fopen(resultsFile, "r");
    if (!fRes) {
        printf("ERROR: Cannot open results file\n");
        return;
    }
    
    // Read and print header
    if (!fgets(header, sizeof(header), fRes)) {
        printf("ERROR: Results file is empty\n");
        fclose(fRes);
        return;
    }
    printf("   Results header: %s", header);
    
    // Create TTree for grain data
    TTree *tree = new TTree("tree", "Grain Data");
    Double_t Area = 0, Circ = 0, Feret = 0, FeretX = 0, FeretY = 0;
    Double_t Round = 0, AR = 0, Solidity = 0;
    
    tree->Branch("Area", &Area, "Area/D");
    tree->Branch("Circ", &Circ, "Circ/D");
    tree->Branch("Feret", &Feret, "Feret/D");
    tree->Branch("FeretX", &FeretX, "FeretX/D");
    tree->Branch("FeretY", &FeretY, "FeretY/D");
    tree->Branch("Round", &Round, "Round/D");
    tree->Branch("AR", &AR, "AR/D");
    tree->Branch("Solidity", &Solidity, "Solidity/D");
    
    // Vectors for statistics
    std::vector<double> vArea, vCirc, vFeret, vFeretX, vFeretY, vRound, vAR, vSolidity;
    
    char buffer[4096];
    Long64_t lineNum = 0;
    Long64_t errorCount = 0;
    
    while (fgets(buffer, sizeof(buffer), fRes)) {
        TString s(buffer);
        s.ReplaceAll("\n", "");
        s.ReplaceAll("\r", "");
        if (s.Length() == 0) continue;
        
        TObjArray *tokens = s.Tokenize(",");
        tokens->SetOwner(kTRUE);
        int nTokens = tokens->GetEntries();
        
        if (nTokens < 16) {
            // Try to parse with fewer columns
            if (nTokens < 10) {
                delete tokens;
                errorCount++;
                continue;
            }
        }
        
        // Safely extract values with bounds checking
        // Column mapping from Results.csv (actual columns)
            // Index  0: (empty label)
            // Index  1: Area
            // Index  2: Mean
            // Index  3: Perim.
            // Index  4: Circ.
            // Index  5: Feret
            // Index  6: IntDen
            // Index  7: RawIntDen
            // Index  8: FeretX
            // Index  9: FeretY
            // Index 10: FeretAngle
            // Index 11: MinFeret
            // Index 12: AR
            // Index 13: Round
            // Index 14: Solidity
            
            // Use direct pixel values and convert
            Double_t area_px = 0;
            Double_t circ_val = 0;
            Double_t feret_px = 0;
            Double_t feretX_px = 0;
            Double_t feretY_px = 0;
            Double_t round_val = 0;
            Double_t ar_val = 0;
            Double_t solidity_val = 0;
            
            // Try different column mappings based on header
            if (nTokens >= 15) {
                // Full format (all 15 columns)
                area_px = ((TObjString*)tokens->At(1))->String().Atof();
                circ_val = ((TObjString*)tokens->At(4))->String().Atof();
                feret_px = ((TObjString*)tokens->At(5))->String().Atof();
                feretX_px = ((TObjString*)tokens->At(8))->String().Atof();
                feretY_px = ((TObjString*)tokens->At(9))->String().Atof();
                ar_val = ((TObjString*)tokens->At(12))->String().Atof();
                round_val = ((TObjString*)tokens->At(13))->String().Atof();
                solidity_val = ((TObjString*)tokens->At(14))->String().Atof();
            } else if (nTokens >= 14) {
                // Partial format (no Solidity)
                area_px = ((TObjString*)tokens->At(1))->String().Atof();
                circ_val = ((TObjString*)tokens->At(4))->String().Atof();
                feret_px = ((TObjString*)tokens->At(5))->String().Atof();
                feretX_px = ((TObjString*)tokens->At(8))->String().Atof();
                feretY_px = ((TObjString*)tokens->At(9))->String().Atof();
                ar_val = ((TObjString*)tokens->At(12))->String().Atof();
                round_val = ((TObjString*)tokens->At(13))->String().Atof();
            } else {
                // Minimal format - try to extract what we can
                area_px = ((TObjString*)tokens->At(1))->String().Atof();
                if (nTokens > 4) circ_val = ((TObjString*)tokens->At(4))->String().Atof();
                if (nTokens > 5) feret_px = ((TObjString*)tokens->At(5))->String().Atof();
                if (nTokens > 8) feretX_px = ((TObjString*)tokens->At(8))->String().Atof();
                if (nTokens > 9) feretY_px = ((TObjString*)tokens->At(9))->String().Atof();
                if (nTokens > 12) ar_val = ((TObjString*)tokens->At(12))->String().Atof();
                if (nTokens > 13) round_val = ((TObjString*)tokens->At(13))->String().Atof();
                if (nTokens > 14) solidity_val = ((TObjString*)tokens->At(14))->String().Atof();
            }
            
            // Convert to microns
            Double_t area_um2 = area_px * k * k;
            Double_t feret_um = feret_px * k;
            Double_t feretX_um = feretX_px * k;
            Double_t feretY_um = feretY_px * k;
            
            if (area_um2 > 0 && area_um2 < 1e6) {  // Sanity check
                vArea.push_back(area_um2);
                vCirc.push_back(circ_val);
                vFeret.push_back(feret_um);
                vFeretX.push_back(feretX_um);
                vFeretY.push_back(feretY_um);
                vRound.push_back(round_val);
                vAR.push_back(ar_val);
                vSolidity.push_back(solidity_val);
                
                // Fill tree
                Area = area_um2;
                Circ = circ_val;
                Feret = feret_um;
                FeretX = feretX_um;
                FeretY = feretY_um;
                Round = round_val;
                AR = ar_val;
                Solidity = solidity_val;
                tree->Fill();
            }

        delete tokens;
        lineNum++;
        
        if (lineNum % 50000 == 0) {
            printf("   Processed %lld lines, %lld grains loaded\r", lineNum, (Long64_t)vArea.size());
            fflush(stdout);
        }
    }
    fclose(fRes);
    
    Long64_t nGrains = tree->GetEntries();
    printf("\n   Processed %lld lines, %lld errors\n", lineNum, errorCount);
    printf("   Loaded %lld grains from results file\n", nGrains);
    printf("   Summary count: %.0f grains\n", summaryCount);
    
    // Validate count
    if (nGrains != (Long64_t)summaryCount) {
        printf("   WARNING: Count mismatch! Results: %lld, Summary: %.0f\n", nGrains, summaryCount);
        printf("   This is expected if the CSV format is different\n");
    }
    
    if (nGrains == 0) {
        printf("ERROR: No grains loaded! Check CSV format and column indices.\n");
        printf("First few lines of the CSV file:\n");
        
        // Print first few lines for debugging
        FILE *fDebug = fopen(resultsFile, "r");
        if (fDebug) {
            char dbuf[4096];
            fgets(dbuf, sizeof(dbuf), fDebug); // header
            for (int i = 0; i < 5 && fgets(dbuf, sizeof(dbuf), fDebug); i++) {
                printf("  Line %d: %s", i+1, dbuf);
            }
            fclose(fDebug);
        }
        delete tree;
        return;
    }
    
    // Calculate statistics from data
    double sumArea = 0, sumCirc = 0, sumFeret = 0, sumRound = 0, sumAR = 0, sumSolidity = 0;
    for (size_t i = 0; i < vArea.size(); i++) {
        sumArea += vArea[i];
        sumCirc += vCirc[i];
        sumFeret += vFeret[i];
        sumRound += vRound[i];
        sumAR += vAR[i];
        sumSolidity += vSolidity[i];
    }
    
    double meanArea = sumArea / nGrains;
    double meanCirc = sumCirc / nGrains;
    double meanFeret = sumFeret / nGrains;
    double meanRound = sumRound / nGrains;
    double meanAR = sumAR / nGrains;
    double meanSolidity = sumSolidity / nGrains;
    
    // Calculate grain density
    double grainDensity = nGrains / fovArea_mm2;
    
    // Calculate centroid from data
    double meanX = 0, meanY = 0;
    for (size_t i = 0; i < vFeretX.size(); i++) {
        meanX += vFeretX[i];
        meanY += vFeretY[i];
    }
    meanX /= nGrains;
    meanY /= nGrains;
    
    // Calculate radial distances
    std::vector<double> radialDist;
    double maxRadius_um = 0;
    for (size_t i = 0; i < nGrains; i++) {
        double dx = vFeretX[i] - meanX;
        double dy = vFeretY[i] - meanY;
        double r_um = sqrt(dx*dx + dy*dy);
        radialDist.push_back(r_um);
        if (r_um > maxRadius_um) maxRadius_um = r_um;
    }
    
    // ====================== CREATE OUTPUT DIRECTORY ======================
    gSystem->mkdir(outDir, kTRUE);
    
    // ====================== SET GLOBAL STYLE ======================
    gStyle->SetOptStat(1111);
    gStyle->SetOptFit(111);
    gStyle->SetPalette(kViridis);
    gStyle->SetTitleFontSize(0.05);
    gStyle->SetTitleFont(42, "xyz");
    gStyle->SetLabelFont(42, "xyz");
    gStyle->SetLabelSize(0.04, "xyz");
    
    // ====================== GENERATE ALL PLOTS ======================
    printf("\n[3] Generating plots for %s...\n", platePrefix.Data());
    
    // -------- 01: Summary Statistics --------
    TCanvas *c1 = new TCanvas("c1", "Summary", 1200, 900);
    TPaveText *pt = new TPaveText(0.08, 0.08, 0.92, 0.92, "NDC");
    pt->SetFillColor(kWhite);
    pt->SetBorderSize(2);
    pt->SetTextFont(42);
    pt->SetTextSize(0.035);
    pt->AddText("=== EMULSION GRAIN ANALYSIS SUMMARY ===");
    pt->AddText(TString::Format("Plate: %s", platePrefix.Data()));
    pt->AddText("");
    pt->AddText("=== DATA VALIDATION ===");
    pt->AddText(TString::Format("Grains from Results: %lld", nGrains));
    pt->AddText(TString::Format("Grains from Summary: %.0f", summaryCount));
    if (nGrains == (Long64_t)summaryCount) {
        pt->AddText("Status: MATCH");
    } else {
        pt->AddText("Status: MISMATCH - Check data integrity");
    }
    pt->AddText("");
    pt->AddText("=== FOV STATISTICS ===");
    pt->AddText(TString::Format("FOV Dimensions: %.2f x %.2f mm", fovW_mm, fovH_mm));
    pt->AddText(TString::Format("FOV Area: %.6f mm2", fovArea_mm2));
    pt->AddText(TString::Format("Grain Density: %.1f grains/mm2", grainDensity));
    pt->AddText(TString::Format("Area Coverage: %.3f %%", summaryPercentArea));
    pt->AddText("");
    pt->AddText("=== GRAIN MORPHOLOGY (from data) ===");
    pt->AddText(TString::Format("Mean Area: %.1f um2", meanArea));
    pt->AddText(TString::Format("Mean Feret Diameter: %.1f um", meanFeret));
    pt->AddText(TString::Format("Mean Circularity: %.3f", meanCirc));
    pt->AddText(TString::Format("Mean Roundness: %.3f", meanRound));
    pt->AddText(TString::Format("Mean Aspect Ratio: %.2f", meanAR));
    pt->AddText(TString::Format("Mean Solidity: %.3f", meanSolidity));
    pt->AddText("");
    pt->AddText("=== SUMMARY FILE METADATA ===");
    pt->AddText(TString::Format("Total Area: %.1f um2", summaryTotalArea));
    pt->AddText(TString::Format("Average Size (summary): %.1f um2", summaryAvgSize));
    pt->AddText(TString::Format("Integrated Density: %.0f", summaryIntDen));
    pt->AddText(TString::Format("Calibration: %.3f um/pixel", k));
    pt->Draw();
    c1->SaveAs(outDir + "/01_Summary.png");
    delete c1;
    printf("   Generated 01_Summary.png\n");
    
    // -------- 02: Area Distribution --------
    TCanvas *c2 = new TCanvas("c2", "Area Distribution", 1200, 900);
    TH1F *hArea = new TH1F("hArea", Form("%s - Area Distribution;Area (um2);Count", platePrefix.Data()), 80, 0, 5000);
    for (size_t i = 0; i < nGrains; i++) hArea->Fill(vArea[i]);
    hArea->SetFillColor(kBlue-9);
    hArea->SetLineColor(kBlue+2);
    hArea->SetLineWidth(2);
    hArea->Draw("HIST");
    c2->SaveAs(outDir + "/02_AreaDistribution.png");
    delete c2;
    printf("   Generated 02_AreaDistribution.png\n");
    
    // -------- 03: Feret Distribution --------
    TCanvas *c3 = new TCanvas("c3", "Feret Distribution", 1200, 900);
    TH1F *hFeret = new TH1F("hFeret", Form("%s - Feret Distribution;Feret Diameter (um);Count", platePrefix.Data()), 80, 0, 150);
    for (size_t i = 0; i < nGrains; i++) hFeret->Fill(vFeret[i]);
    hFeret->SetFillColor(kRed-9);
    hFeret->SetLineColor(kRed+2);
    hFeret->SetLineWidth(2);
    hFeret->Draw("HIST");
    c3->SaveAs(outDir + "/03_FeretDistribution.png");
    delete c3;
    printf("   Generated 03_FeretDistribution.png\n");
    
    // -------- 04: Circularity & Roundness --------
    TCanvas *c4 = new TCanvas("c4", "Circularity & Roundness", 1600, 800);
    c4->Divide(2,1);
    c4->cd(1);
    TH1F *hCirc = new TH1F("hCirc", Form("%s - Circularity Distribution;Circularity;Count", platePrefix.Data()), 60, 0.4, 1.0);
    for (size_t i = 0; i < nGrains; i++) if (vCirc[i] <= 1) hCirc->Fill(vCirc[i]);
    hCirc->SetFillColor(kMagenta-6);
    hCirc->SetLineColor(kMagenta+2);
    hCirc->Draw("HIST");
    c4->cd(2);
    TH1F *hRound = new TH1F("hRound", Form("%s - Roundness Distribution;Roundness;Count", platePrefix.Data()), 60, 0.2, 1.0);
    for (size_t i = 0; i < nGrains; i++) if (vRound[i] <= 1) hRound->Fill(vRound[i]);
    hRound->SetFillColor(kYellow-6);
    hRound->SetLineColor(kYellow+2);
    hRound->Draw("HIST");
    c4->SaveAs(outDir + "/04_CircularityRoundness.png");
    delete c4;
    printf("   Generated 04_CircularityRoundness.png\n");
    
    // Compute actual plate extent from grain positions
    double minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;
    for (size_t i = 0; i < nGrains; i++) {
        if (vFeretX[i] < minX) minX = vFeretX[i];
        if (vFeretX[i] > maxX) maxX = vFeretX[i];
        if (vFeretY[i] < minY) minY = vFeretY[i];
        if (vFeretY[i] > maxY) maxY = vFeretY[i];
    }
    // Use the larger of data-extent and summary-derived extent
    double dataArea_um2 = (maxX - minX) * (maxY - minY);
    double extentW_mm, extentH_mm;
    if (dataArea_um2 >= plateArea_um2) {
        // Data covers the full plate
        extentW_mm = (maxX - minX) / 1000.0;
        extentH_mm = (maxY - minY) / 1000.0;
    } else {
        // Scale up to match summary plate area
        double aspect = (double)(maxX - minX) / (maxY - minY);
        extentH_mm = sqrt(plateArea_um2 / aspect) / 1000.0;
        extentW_mm = extentH_mm * aspect;
    }
    printf("   Plate area from summary: %.0f um2, data extent: %.0f x %.0f um\n",
           plateArea_um2, maxX - minX, maxY - minY);
    
    // -------- 05: 2D Density Heatmap --------
    TCanvas *c5 = new TCanvas("c5", "2D Heatmap", 1400, 1200);
    TH2F *hHeat = new TH2F("hHeat", Form("%s - 2D Density Heatmap;X Position (mm);Y Position (mm);Grains per Bin", platePrefix.Data()),
                           60, 0, extentW_mm, 60, 0, extentH_mm);
    for (size_t i = 0; i < nGrains; i++) {
        hHeat->Fill((vFeretX[i] - minX)/1000.0, (vFeretY[i] - minY)/1000.0);
    }
    hHeat->SetStats(0);
    hHeat->Draw("COLZ");
    gPad->SetRightMargin(0.15);
    c5->SaveAs(outDir + "/05_2DDensityHeatmap.png");
    delete c5;
    printf("   Generated 05_2DDensityHeatmap.png\n");
    
    // -------- 06: 2D Scatter Plot --------
    TCanvas *c6 = new TCanvas("c6", "Scatter Plot", 1400, 1200);
    TH2F *hScat = new TH2F("hScat", Form("%s - Grain Positions (Scatter);X (mm);Y (mm)", platePrefix.Data()),
                           120, 0, extentW_mm, 120, 0, extentH_mm);
    for (size_t i = 0; i < nGrains; i++) {
        hScat->Fill((vFeretX[i] - minX)/1000.0, (vFeretY[i] - minY)/1000.0);
    }
    hScat->SetStats(0);
    hScat->Draw("COLZ");
    c6->SaveAs(outDir + "/06_2DScatterPlot.png");
    delete c6;
    printf("   Generated 06_2DScatterPlot.png\n");
    
    // -------- 07: Radial Density Profile --------
    TCanvas *c7 = new TCanvas("c7", "Radial Density", 1200, 900);
    TH1F *hRad = new TH1F("hRad", Form("%s - Radial Density Profile;Radius (mm);Density (grains/mm2)", platePrefix.Data()), 
                          40, 0, maxRadius_um/1000.0);
    for (int i = 1; i <= 40; i++) {
        double r1_mm = hRad->GetBinLowEdge(i);
        double r2_mm = hRad->GetBinLowEdge(i+1);
        double r1_um = r1_mm * 1000.0;
        double r2_um = r2_mm * 1000.0;
        double area_mm2 = TMath::Pi() * (r2_mm*r2_mm - r1_mm*r1_mm);
        double cnt = 0;
        for (size_t j = 0; j < nGrains; j++) {
            if (radialDist[j] >= r1_um && radialDist[j] < r2_um) cnt++;
        }
        hRad->SetBinContent(i, area_mm2 > 0 ? cnt / area_mm2 : 0);
    }
    hRad->SetFillColor(kAzure-5);
    hRad->SetLineColor(kAzure+2);
    hRad->SetLineWidth(2);
    hRad->Draw("HIST");
    c7->SaveAs(outDir + "/07_RadialDensityProfile.png");
    delete c7;
    printf("   Generated 07_RadialDensityProfile.png\n");
    
    // -------- 08: Differential Flux --------
    TCanvas *c8 = new TCanvas("c8", "Differential Flux", 1200, 900);
    TH1F *hDiff = new TH1F("hDiff", Form("%s - Differential Flux;Radius (mm);Grains per Annulus", platePrefix.Data()),
                           40, 0, maxRadius_um/1000.0);
    for (int i = 1; i <= 40; i++) {
        double r1_mm = hDiff->GetBinLowEdge(i);
        double r2_mm = hDiff->GetBinLowEdge(i+1);
        double r1_um = r1_mm * 1000.0;
        double r2_um = r2_mm * 1000.0;
        double cnt = 0;
        for (size_t j = 0; j < nGrains; j++) {
            if (radialDist[j] >= r1_um && radialDist[j] < r2_um) cnt++;
        }
        hDiff->SetBinContent(i, cnt);
    }
    hDiff->SetFillColor(kTeal-5);
    hDiff->SetLineColor(kTeal+2);
    hDiff->SetLineWidth(2);
    hDiff->Draw("HIST");
    c8->SaveAs(outDir + "/08_DifferentialFlux.png");
    delete c8;
    printf("   Generated 08_DifferentialFlux.png\n");
    
    // -------- 09: Cumulative Count --------
    TCanvas *c9 = new TCanvas("c9", "Cumulative Count", 1200, 900);
    TH1F *hCum = new TH1F("hCum", Form("%s - Cumulative Grain Count;Radius (mm);Cumulative Grains", platePrefix.Data()),
                          40, 0, maxRadius_um/1000.0);
    for (int i = 1; i <= 40; i++) {
        double r_mm = hCum->GetBinLowEdge(i+1);
        double r_um = r_mm * 1000.0;
        double cnt = 0;
        for (size_t j = 0; j < nGrains; j++) {
            if (radialDist[j] < r_um) cnt++;
        }
        hCum->SetBinContent(i, cnt);
    }
    hCum->SetFillColor(kGreen-9);
    hCum->SetLineColor(kGreen+2);
    hCum->SetLineWidth(2);
    hCum->Draw("HIST");
    c9->SaveAs(outDir + "/09_CumulativeCount.png");
    delete c9;
    printf("   Generated 09_CumulativeCount.png\n");
    
    // -------- 10: Local Fluence Map --------
    TCanvas *c10 = new TCanvas("c10", "Local Fluence", 1200, 900);
    TH2F *hFlu = new TH2F("hFlu", Form("%s - Local Fluence Map;X (mm);Y (mm);Grains per Cell", platePrefix.Data()),
                          10, 0, extentW_mm, 10, 0, extentH_mm);
    for (size_t i = 0; i < nGrains; i++) {
        hFlu->Fill((vFeretX[i] - minX)/1000.0, (vFeretY[i] - minY)/1000.0);
    }
    hFlu->SetStats(0);
    hFlu->Draw("COLZ TEXT");
    c10->SaveAs(outDir + "/10_LocalFluenceMap.png");
    delete c10;
    printf("   Generated 10_LocalFluenceMap.png\n");
    
    // -------- 11: Horizontal Profile --------
    TCanvas *c11 = new TCanvas("c11", "Horizontal Profile", 1200, 900);
    TH1F *hHX = new TH1F("hHX", Form("%s - Horizontal Profile;X Position (mm);Count", platePrefix.Data()), 80, 0, extentW_mm);
    for (size_t i = 0; i < nGrains; i++) hHX->Fill((vFeretX[i] - minX)/1000.0);
    hHX->SetFillColor(kBlue-9);
    hHX->SetLineColor(kBlue+2);
    hHX->SetLineWidth(2);
    hHX->Draw("HIST");
    c11->SaveAs(outDir + "/11_HorizontalSlitProfile.png");
    delete c11;
    printf("   Generated 11_HorizontalSlitProfile.png\n");
    
    // -------- 12: Vertical Profile --------
    TCanvas *c12 = new TCanvas("c12", "Vertical Profile", 1200, 900);
    TH1F *hHY = new TH1F("hHY", Form("%s - Vertical Profile;Y Position (mm);Count", platePrefix.Data()), 80, 0, extentH_mm);
    for (size_t i = 0; i < nGrains; i++) hHY->Fill((vFeretY[i] - minY)/1000.0);
    hHY->SetFillColor(kRed-9);
    hHY->SetLineColor(kRed+2);
    hHY->SetLineWidth(2);
    hHY->Draw("HIST");
    c12->SaveAs(outDir + "/12_VerticalSlitProfile.png");
    delete c12;
    printf("   Generated 12_VerticalSlitProfile.png\n");
    
    // -------- 13: XY Profile Comparison --------
    TCanvas *c13 = new TCanvas("c13", "XY Comparison", 1400, 1000);
    TH1F *hHXNorm = (TH1F*)hHX->Clone();
    TH1F *hHYNorm = (TH1F*)hHY->Clone();
    if (hHXNorm->GetMaximum() > 0) hHXNorm->Scale(1.0 / hHXNorm->GetMaximum());
    if (hHYNorm->GetMaximum() > 0) hHYNorm->Scale(1.0 / hHYNorm->GetMaximum());
    hHXNorm->SetTitle(Form("%s - X vs Y Profile Comparison;Position (mm);Normalized Count", platePrefix.Data()));
    hHXNorm->SetLineColor(kBlue+2);
    hHXNorm->SetLineWidth(2);
    hHYNorm->SetLineColor(kRed+2);
    hHYNorm->SetLineWidth(2);
    hHXNorm->Draw("HIST");
    hHYNorm->Draw("HISTSAME");
    
    TLegend *leg13 = new TLegend(0.7, 0.7, 0.88, 0.88);
    leg13->AddEntry(hHXNorm, "X Profile", "l");
    leg13->AddEntry(hHYNorm, "Y Profile", "l");
    leg13->Draw();
    
    c13->SaveAs(outDir + "/13_XYProfileComparison.png");
    delete c13;
    delete hHXNorm;
    delete hHYNorm;
    printf("   Generated 13_XYProfileComparison.png\n");
    
    // ====================== CLEANUP ======================
    printf("\n[4] Cleanup...\n");
    delete tree;
    printf("   Done with %s\n", platePrefix.Data());
}

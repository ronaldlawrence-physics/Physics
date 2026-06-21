#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TLine.h>
#include <TLatex.h>
#include <TStyle.h>
#include <TString.h>
#include <TMath.h>
#include <TGraphErrors.h>
#include <TProfile.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cmath>

using namespace std;

struct Particle {
    double area, xm, ym, perim, circ, ar, round, solidity;
};

struct TiffDims {
    int wPx, hPx;
    double wUm, hUm;
};

vector<Particle> readCSV(const TString &filePath) {
    vector<Particle> data;
    ifstream f(filePath.Data());
    if (!f.is_open()) {
        cerr << "Cannot open " << filePath << endl;
        return data;
    }
    string line;
    getline(f, line);
    while (getline(f, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string token;
        getline(ss, token, ',');
        getline(ss, token, ',');
        Particle p;
        getline(ss, token, ','); p.area = stod(token);
        getline(ss, token, ',');
        getline(ss, token, ','); p.xm = stod(token);
        getline(ss, token, ','); p.ym = stod(token);
        getline(ss, token, ','); p.perim = stod(token);
        getline(ss, token, ','); p.circ = stod(token);
        getline(ss, token, ',');
        getline(ss, token, ',');
        getline(ss, token, ','); p.ar = stod(token);
        getline(ss, token, ','); p.round = stod(token);
        getline(ss, token, ','); p.solidity = stod(token);
        data.push_back(p);
    }
    return data;
}

TiffDims getTiffDims(const TString &tiffPath) {
    TiffDims d = {0, 0, 0, 0};
    FILE *f = fopen(tiffPath.Data(), "rb");
    if (!f) return d;
    unsigned char hdr[8];
    if (fread(hdr, 1, 8, f) != 8) { fclose(f); return d; }
    bool bigEndian = (hdr[0] == 'M' && hdr[1] == 'M');
    bool littleEndian = (hdr[0] == 'I' && hdr[1] == 'I');
    if (!bigEndian && !littleEndian) { fclose(f); return d; }
    int ifdOffset;
    if (littleEndian)
        ifdOffset = hdr[4] | (hdr[5] << 8) | (hdr[6] << 16) | (hdr[7] << 24);
    else
        ifdOffset = (hdr[4] << 24) | (hdr[5] << 16) | (hdr[6] << 8) | hdr[7];
    unsigned char ifdBuf[4096];
    fseek(f, ifdOffset, SEEK_SET);
    int toRead = (sizeof(ifdBuf) < 4096) ? sizeof(ifdBuf) : 4096;
    int nread = fread(ifdBuf, 1, toRead, f);
    fclose(f);
    if (nread < 2) return d;
    int numEntries;
    if (littleEndian)
        numEntries = ifdBuf[0] | (ifdBuf[1] << 8);
    else
        numEntries = (ifdBuf[0] << 8) | ifdBuf[1];
    int w = 0, h = 0;
    for (int i = 0; i < numEntries; i++) {
        int off = 2 + i * 12;
        if (off + 12 > nread) break;
        int tag, type;
        if (littleEndian) {
            tag = ifdBuf[off] | (ifdBuf[off+1] << 8);
            type = ifdBuf[off+2] | (ifdBuf[off+3] << 8);
        } else {
            tag = (ifdBuf[off] << 8) | ifdBuf[off+1];
            type = (ifdBuf[off+2] << 8) | ifdBuf[off+3];
        }
        int val = 0;
        if (type == 3) {
            if (littleEndian)
                val = ifdBuf[off+8] | (ifdBuf[off+9] << 8);
            else
                val = (ifdBuf[off+8] << 8) | ifdBuf[off+9];
        } else if (type == 4) {
            if (littleEndian)
                val = ifdBuf[off+8] | (ifdBuf[off+9] << 8) | (ifdBuf[off+10] << 16) | (ifdBuf[off+11] << 24);
            else
                val = (ifdBuf[off+8] << 24) | (ifdBuf[off+9] << 16) | (ifdBuf[off+10] << 8) | ifdBuf[off+11];
        }
        if (tag == 256) w = val;
        if (tag == 257) h = val;
    }
    if (w > 0 && h > 0) {
        d.wPx = w;
        d.hPx = h;
        double scale = 1.717;
        d.wUm = w / scale;
        d.hUm = h / scale;
    }
    return d;
}

void GeneratePlots() {
    gStyle->SetOptStat(0);
    gStyle->SetPalette(kViridis);

    const TString baseDir = "/home/ronald/JINR/LHEP/Data/Xe_280Mev_2026/Stack_03";
    const TString resultsDir = baseDir + "/Results_Files";
    const TString emulDir = baseDir + "/Emulsion_Files";
    const TString plotsDir = baseDir + "/Plots";

    gSystem->Exec("mkdir -p " + plotsDir);

    vector<TString> plates = {
        "Plate_01_p1", "Plate_02_p1", "Plate_03_p1", "Plate_04_p1",
        "Plate_05_p1", "Plate_06_p1", "Plate_07_p1", "Plate_08_p1",
        "Plate_09_p1", "Plate_10_p1"
    };

    for (const auto &plate : plates) {
        TString csvFile = resultsDir + "/" + plate + "_Results.csv";
        TString tifFile = emulDir + "/" + plate + ".tif";
        TiffDims plateDims = getTiffDims(tifFile);
        if (plateDims.wPx == 0 || plateDims.hPx == 0) {
            cerr << "Cannot get TIFF dimensions for " << plate << endl;
            continue;
        }

        vector<Particle> particles = readCSV(csvFile);
        if (particles.empty()) {
            cout << "No data for " << plate << endl;
            continue;
        }

        TString outDir = plotsDir + "/" + plate;
        gSystem->Exec("mkdir -p " + outDir);
        cout << "Processing " << plate << " (" << particles.size() << " particles, "
             << "img=" << plateDims.wPx << "x" << plateDims.hPx << " px = "
             << plateDims.wUm << "x" << plateDims.hUm << " um)" << endl;

        double xMin = 1e9, xMax = -1e9, yMin = 1e9, yMax = -1e9;
        double areaMin = 1e9, areaMax = -1e9;
        for (auto &p : particles) {
            if (p.xm < xMin) xMin = p.xm;
            if (p.xm > xMax) xMax = p.xm;
            if (p.ym < yMin) yMin = p.ym;
            if (p.ym > yMax) yMax = p.ym;
            if (p.area < areaMin) areaMin = p.area;
            if (p.area > areaMax) areaMax = p.area;
        }
        double xRange = xMax - xMin;
        double yRange = yMax - yMin;
        double cx = (xMin + xMax) / 2;
        double cy = (yMin + yMax) / 2;
        double maxRad = sqrt(xRange * xRange + yRange * yRange) / 2;
        double extentW_mm = xRange / 1000.0;
        double extentH_mm = yRange / 1000.0;

        // ===== 01_Summary.png =====
        TCanvas c1("c1", "", 1200, 800);
        c1.SetWindowSize(1200, 800);
        c1.SetFillColor(0);
        TPaveText pt(0.05, 0.05, 0.95, 0.95);
        pt.SetFillColor(0);
        pt.SetBorderSize(1);
        pt.SetTextFont(42);
        pt.SetTextSize(0.035);
        TString label = plate;
        label.ReplaceAll("_", "\\_");
        pt.AddText(Form("Plate: %s", label.Data()));
        pt.AddText(Form("Particles detected: %zu", particles.size()));
        pt.AddText(Form("Plate size: %.0f x %.0f um  (%d x %d px)",
                        plateDims.wUm, plateDims.hUm, plateDims.wPx, plateDims.hPx));

        double sumArea = 0, sumCirc = 0, sumRound = 0, sumSolid = 0;
        double sumPerim = 0, sumFeret = 0;
        double minArea = 1e9, maxArea = -1e9;
        for (auto &p : particles) {
            sumArea += p.area;
            sumCirc += p.circ;
            sumRound += p.round;
            sumSolid += p.solidity;
            sumPerim += p.perim;
            double dEff = 2 * sqrt(p.area / TMath::Pi());
            sumFeret += dEff;
            if (p.area < minArea) minArea = p.area;
            if (p.area > maxArea) maxArea = p.area;
        }
        double avgArea = sumArea / particles.size();
        double stdDev = 0;
        for (auto &p : particles)
            stdDev += (p.area - avgArea) * (p.area - avgArea);
        stdDev = sqrt(stdDev / particles.size());

        pt.AddText("");
        pt.AddText(Form("Area: total=%.1f um^2  avg=%.3f um^2", sumArea, avgArea));
        pt.AddText(Form("Area: min=%.3f  max=%.3f  std=%.3f um^2", minArea, maxArea, stdDev));
        pt.AddText(Form("Avg Circularity=%.4f  Avg Roundness=%.4f", sumCirc / particles.size(), sumRound / particles.size()));
        pt.AddText(Form("Avg Solidity=%.4f  Avg Perimeter=%.3f um", sumSolid / particles.size(), sumPerim / particles.size()));
        pt.AddText(Form("Avg Eff. Feret=%.3f um", sumFeret / particles.size()));
        pt.AddText(Form("Data X range: [%.0f, %.0f]  Y range: [%.0f, %.0f]", xMin, xMax, yMin, yMax));
        pt.Draw();
        c1.SaveAs(outDir + "/01_Summary.png");

        TString pLabel = plate;
        pLabel.ReplaceAll("_", "\\_");

        // ===== 02_AreaDistribution.png =====
        TCanvas c2("c2", "", 1000, 700);
        TH1F *hArea = new TH1F("hArea", Form("%s - Area Distribution;Area (um^2);Count", pLabel.Data()),
                                100, areaMin, areaMax);
        for (auto &p : particles) hArea->Fill(p.area);
        hArea->SetFillColor(kBlue - 9);
        hArea->SetLineColor(kBlue + 2);
        hArea->SetLineWidth(2);
        hArea->Draw("hist");
        c2.SaveAs(outDir + "/02_AreaDistribution.png");
        delete hArea;

        // ===== 03_FeretDistribution.png =====
        TCanvas c3("c3", "", 1000, 700);
        TH1F *hFeret = new TH1F("hFeret", Form("%s - Feret Distribution;Effective Diameter (um);Count", pLabel.Data()),
                                100, 0, 2 * sqrt(areaMax / TMath::Pi()));
        for (auto &p : particles) hFeret->Fill(2 * sqrt(p.area / TMath::Pi()));
        hFeret->SetFillColor(kRed - 9);
        hFeret->SetLineColor(kRed + 2);
        hFeret->SetLineWidth(2);
        hFeret->Draw("hist");
        c3.SaveAs(outDir + "/03_FeretDistribution.png");
        delete hFeret;

        // ===== 04_CircularityRoundness.png (two panels) =====
        TCanvas c4("c4", "", 1600, 800);
        c4.Divide(2, 1);
        c4.cd(1);
        TH1F *hCirc = new TH1F("hCirc", Form("%s - Circularity;Circularity;Count", pLabel.Data()), 50, 0, 1);
        for (auto &p : particles) hCirc->Fill(p.circ);
        hCirc->SetFillColor(kBlue - 9);
        hCirc->SetLineColor(kBlue + 2);
        hCirc->SetLineWidth(2);
        hCirc->Draw("hist");
        c4.cd(2);
        TH1F *hRound = new TH1F("hRound", Form("%s - Roundness;Roundness;Count", pLabel.Data()), 50, 0, 1);
        for (auto &p : particles) hRound->Fill(p.round);
        hRound->SetFillColor(kRed - 9);
        hRound->SetLineColor(kRed + 2);
        hRound->SetLineWidth(2);
        hRound->Draw("hist");
        c4.SaveAs(outDir + "/04_CircularityRoundness.png");
        delete hCirc;
        delete hRound;

        // ===== 05_2DDensityHeatmap.png (full plate) =====
        TCanvas c5("c5", "", 1400, 1000);
        TH2F *hDens = new TH2F("hDens", Form("%s - 2D Density Heatmap;X (mm);Y (mm);Grains per Bin", pLabel.Data()),
                                60, 0, extentW_mm, 60, 0, extentH_mm);
        for (auto &p : particles) hDens->Fill((p.xm - xMin) / 1000.0, (p.ym - yMin) / 1000.0);
        hDens->Draw("colz");
        c5.SaveAs(outDir + "/05_2DDensityHeatmap.png");
        delete hDens;

        // ===== 06_2DScatterPlot.png (spots in plate) =====
        TCanvas c6("c6", "", 1400, 1000);
        TH2F *hScat = new TH2F("hScat", Form("%s - 2D Scatter Plot;X (mm);Y (mm);Grains per Bin", pLabel.Data()),
                                120, 0, extentW_mm, 120, 0, extentH_mm);
        for (auto &p : particles) hScat->Fill((p.xm - xMin) / 1000.0, (p.ym - yMin) / 1000.0);
        hScat->Draw("colz");
        c6.SaveAs(outDir + "/06_2DScatterPlot.png");
        delete hScat;

        // ===== 07_RadialDensityProfile.png =====
        TCanvas c7("c7", "", 1000, 700);
        int rBins = 100;
        TH1F *hRad = new TH1F("hRad", Form("%s - Radial Profile;Radial Distance from Center (um);Count / bin", pLabel.Data()),
                              rBins, 0, maxRad);
        for (auto &p : particles) {
            double r = sqrt((p.xm - cx) * (p.xm - cx) + (p.ym - cy) * (p.ym - cy));
            hRad->Fill(r);
        }
        TH1F *hRadProf = new TH1F("hRadProf", Form("%s - Radial Density;Radial Distance (um);Density (count/um^2)", pLabel.Data()),
                                  rBins, 0, maxRad);
        double dr = maxRad / rBins;
        for (int i = 1; i <= rBins; i++) {
            double rCenter = (i - 0.5) * dr;
            double annulusArea = 2 * TMath::Pi() * rCenter * dr;
            if (annulusArea > 0)
                hRadProf->SetBinContent(i, hRad->GetBinContent(i) / annulusArea);
        }
        hRadProf->SetFillColor(kAzure - 5);
        hRadProf->SetLineColor(kAzure + 2);
        hRadProf->SetLineWidth(2);
        hRadProf->Draw("hist");
        c7.SaveAs(outDir + "/07_RadialDensityProfile.png");
        delete hRad;
        delete hRadProf;

        // ===== 08_DifferentialFlux.png =====
        TCanvas c8("c8", "", 1000, 700);
        TH1F *hFlux = new TH1F("hFlux", Form("%s - Differential Flux;Area (um^2);dN/dA", pLabel.Data()),
                                100, areaMin, areaMax);
        hFlux->Sumw2();
        for (auto &p : particles) hFlux->Fill(p.area);
        for (int i = 1; i <= hFlux->GetNbinsX(); i++) {
            double bw = hFlux->GetBinWidth(i);
            if (bw > 0) hFlux->SetBinContent(i, hFlux->GetBinContent(i) / bw);
        }
        hFlux->SetFillColor(kTeal - 5);
        hFlux->SetLineColor(kTeal + 2);
        hFlux->SetLineWidth(2);
        hFlux->Draw("hist");
        c8.SaveAs(outDir + "/08_DifferentialFlux.png");
        delete hFlux;

        // ===== 09_CumulativeCount.png =====
        TCanvas c9("c9", "", 1000, 700);
        vector<double> areas;
        for (auto &p : particles) areas.push_back(p.area);
        sort(areas.begin(), areas.end());
        TGraph *gCum = new TGraph();
        double nTot = areas.size();
        for (size_t i = 0; i < areas.size(); i++)
            gCum->SetPoint(i, areas[i], (i + 1.0) / nTot * 100);
        gCum->SetFillColor(kGreen - 9);
        gCum->SetLineColor(kGreen + 2);
        gCum->SetLineWidth(2);
        gCum->Draw("alf");
        gCum->GetHistogram()->SetXTitle("Area (um^2)");
        gCum->GetHistogram()->SetYTitle("Cumulative (%)");
        gCum->SetTitle(Form("%s - Cumulative Count", pLabel.Data()));
        c9.SaveAs(outDir + "/09_CumulativeCount.png");
        delete gCum;

        // ===== 10_LocalFluenceMap.png (tiled full plate) =====
        TCanvas c10("c10", "", 1200, 800);
        TH2F *hFluence = new TH2F("hFluence", Form("%s - Local Fluence Map;X (mm);Y (mm);Grains per Cell", pLabel.Data()),
                                   10, 0, extentW_mm, 10, 0, extentH_mm);
        for (auto &p : particles) hFluence->Fill((p.xm - xMin) / 1000.0, (p.ym - yMin) / 1000.0);
        hFluence->Draw("colz text");
        c10.SaveAs(outDir + "/10_LocalFluenceMap.png");
        delete hFluence;

        // ===== 11_HorizontalSlitProfile.png =====
        TCanvas c11("c11", "", 1000, 700);
        int slitBins = 100;
        TH1F *hXProf = new TH1F("hXProf", Form("%s - Horizontal Profile;X (mm);Count", pLabel.Data()),
                                 slitBins, 0, extentW_mm);
        for (auto &p : particles) hXProf->Fill((p.xm - xMin) / 1000.0);
        hXProf->SetFillColor(kBlue - 9);
        hXProf->SetLineColor(kBlue + 2);
        hXProf->SetLineWidth(2);
        hXProf->Draw("hist");
        c11.SaveAs(outDir + "/11_HorizontalSlitProfile.png");
        delete hXProf;

        // ===== 12_VerticalSlitProfile.png =====
        TCanvas c12("c12", "", 1000, 700);
        TH1F *hYProf = new TH1F("hYProf", Form("%s - Vertical Profile;Y (mm);Count", pLabel.Data()),
                                 slitBins, 0, extentH_mm);
        for (auto &p : particles) hYProf->Fill((p.ym - yMin) / 1000.0);
        hYProf->SetFillColor(kRed - 9);
        hYProf->SetLineColor(kRed + 2);
        hYProf->SetLineWidth(2);
        hYProf->Draw("hist");
        c12.SaveAs(outDir + "/12_VerticalSlitProfile.png");
        delete hYProf;

        // ===== 13_XYProfileComparison.png =====
        TCanvas c13("c13", "", 1400, 1000);
        int compBins = 80;
        TH1F *hX = new TH1F("hX", Form("%s - X vs Y Profile Comparison;Fractional Position;Normalized Count", pLabel.Data()),
                             compBins, 0, 1);
        TH1F *hY = new TH1F("hY", Form("%s - X vs Y Profile Comparison;Fractional Position;Normalized Count", pLabel.Data()),
                             compBins, 0, 1);
        for (auto &p : particles) {
            hX->Fill((p.xm - xMin) / xRange);
            hY->Fill((p.ym - yMin) / yRange);
        }
        if (hX->GetSum() > 0) hX->Scale(1.0 / hX->GetSum());
        if (hY->GetSum() > 0) hY->Scale(1.0 / hY->GetSum());
        double pMax = max(hX->GetMaximum(), hY->GetMaximum());
        if (pMax > 0) hX->SetMaximum(pMax * 1.15);
        hX->SetFillColorAlpha(kBlue - 9, 0.3);
        hX->SetLineColor(kBlue + 2);
        hX->SetLineWidth(2);
        hY->SetFillColorAlpha(kRed - 9, 0.3);
        hY->SetLineColor(kRed + 2);
        hY->SetLineWidth(2);
        hX->Draw("hist");
        hY->Draw("hist same");
        TLegend *leg = new TLegend(0.6, 0.75, 0.88, 0.88);
        leg->AddEntry(hX, "X Profile", "l");
        leg->AddEntry(hY, "Y Profile", "l");
        leg->Draw();
        c13.SaveAs(outDir + "/13_XYProfileComparison.png");
        delete hX;
        delete hY;
        delete leg;
    }

    cout << "\n=== All plots generated in " << plotsDir << " ===" << endl;
}

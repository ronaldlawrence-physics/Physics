#include <TGraph.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TH1F.h>
#include <TPaveText.h>
#include <TStyle.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>

void plot_class2() {
    gStyle->SetOptStat(0);
    gStyle->SetPadGridX(1);
    gStyle->SetPadGridY(1);

    TString base = "/home/ronald/JINR/LHEP/Data/Xe_280Mev_2026";
    Int_t col[3] = {kBlue, kRed, kGreen+2};
    Int_t mkr[3] = {20, 21, 22};
    TString sn[3] = {"Stack_01", "Stack_02", "Stack_03"};

    // ===== Read class data =====
    std::map<TString, std::map<Float_t, Float_t>> frac_map;   // stack -> depth -> frac
    std::map<TString, std::map<Float_t, Float_t>> area_map;   // stack -> depth -> area_mean

    std::ifstream fcin((base + "/analysis/classes_root.dat").Data());
    if (!fcin.is_open()) { printf("ERROR: cannot open classes_root.dat\n"); return; }
    std::string line;
    std::getline(fcin, line);
    while (std::getline(fcin, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::stringstream ss(line);
        std::string s, p; Int_t c; Float_t d, f, a;
        ss >> s >> p >> d >> c >> f >> a;
        if (c == 2) {
            frac_map[TString(s)][d] = f;
            area_map[TString(s)][d] = a;
        }
    }
    fcin.close();

    // ===== Read Bragg data =====
    std::map<TString, std::map<Float_t, Float_t>> td_map;     // stack -> depth -> track density
    std::map<TString, std::map<Float_t, Float_t>> ed_map;     // stack -> depth -> energy dep

    std::ifstream fin((base + "/analysis/bragg_root.dat").Data());
    if (!fin.is_open()) { printf("ERROR: cannot open bragg_root.dat\n"); return; }
    std::getline(fin, line);
    while (std::getline(fin, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::stringstream ss(line);
        std::string s, p; Float_t d, tden, tdpcv, edep, aa; Int_t cyc;
        ss >> s >> p >> d >> cyc >> tden >> tdpcv >> edep >> aa;
        td_map[TString(s)][d] = tden;
        ed_map[TString(s)][d] = edep;
    }
    fin.close();

    // ===== Build graphs =====
    TGraph *g_frac[3], *g_area[3], *g_contrib[3];

    for (Int_t s = 0; s < 3; s++) {
        TString name = sn[s];
        auto &deps_f = frac_map[name];
        std::vector<Float_t> xf, yf, xa, ya, xc, yc;
        for (auto &kv : deps_f) {
            Float_t d = kv.first;
            xf.push_back(d); yf.push_back(kv.second);
            if (area_map[name].count(d)) {
                xa.push_back(d); ya.push_back(area_map[name][d]);
            }
            if (td_map[name].count(d)) {
                xc.push_back(d); yc.push_back(kv.second * td_map[name][d]);
            }
        }
        std::sort(xf.begin(), xf.end());
        // Re-sort by depth
        // (map is already sorted by key)
        g_frac[s] = new TGraph(xf.size(), &xf[0], &yf[0]);
        g_frac[s]->SetLineWidth(2); g_frac[s]->SetLineColor(col[s]);
        g_frac[s]->SetMarkerStyle(mkr[s]); g_frac[s]->SetMarkerColor(col[s]);

        g_area[s] = new TGraph(xa.size(), &xa[0], &ya[0]);
        g_area[s]->SetLineWidth(2); g_area[s]->SetLineColor(col[s]);
        g_area[s]->SetMarkerStyle(mkr[s]); g_area[s]->SetMarkerColor(col[s]);

        g_contrib[s] = new TGraph(xc.size(), &xc[0], &yc[0]);
        g_contrib[s]->SetLineWidth(2); g_contrib[s]->SetLineColor(col[s]);
        g_contrib[s]->SetMarkerStyle(mkr[s]); g_contrib[s]->SetMarkerColor(col[s]);
    }

    // ===== Canvas: 3 panels =====
    TCanvas *c = new TCanvas("c", "Class 2 Analysis", 1600, 1200);
    c->Divide(2, 2);

    // Panel 1: Class 2 fraction
    c->cd(1);
    Float_t max_f = 0;
    for (Int_t s = 0; s < 3; s++)
        for (int i = 0; i < g_frac[s]->GetN(); i++)
            max_f = TMath::Max(max_f, (Float_t)g_frac[s]->GetY()[i]);
    TH1F *hf = gPad->DrawFrame(0, 0, 20, TMath::Max(max_f*1.3, 0.5),
        ";Depth (mm);Class 2 Fraction");
    TLegend *l1 = new TLegend(0.5, 0.7, 0.88, 0.88);
    for (Int_t s = 0; s < 3; s++) {
        g_frac[s]->Draw("CP");
        l1->AddEntry(g_frac[s], sn[s], "lp");
    }
    l1->Draw();

    // Panel 2: Class 2 mean area
    c->cd(2);
    Float_t max_a = 0;
    for (Int_t s = 0; s < 3; s++)
        for (int i = 0; i < g_area[s]->GetN(); i++)
            max_a = TMath::Max(max_a, (Float_t)g_area[s]->GetY()[i]);
    TH1F *ha = gPad->DrawFrame(0, 0, 20, max_a*1.2,
        ";Depth (mm);Class 2 Mean Grain Area (#mum^{2})");
    TLegend *l2 = new TLegend(0.5, 0.7, 0.88, 0.88);
    for (Int_t s = 0; s < 3; s++) {
        g_area[s]->Draw("CP");
        l2->AddEntry(g_area[s], sn[s], "lp");
    }
    l2->Draw();

    // Panel 3: Class 2 weighted contribution
    c->cd(3);
    Float_t max_c = 0;
    for (Int_t s = 0; s < 3; s++)
        for (int i = 0; i < g_contrib[s]->GetN(); i++)
            max_c = TMath::Max(max_c, (Float_t)g_contrib[s]->GetY()[i]);
    TH1F *hc = gPad->DrawFrame(0, 0, 20, max_c*1.2,
        ";Depth (mm);Class 2 Weighted Density (tracks/mm^{2})");
    TLegend *l3 = new TLegend(0.5, 0.7, 0.88, 0.88);
    for (Int_t s = 0; s < 3; s++) {
        g_contrib[s]->Draw("CP");
        l3->AddEntry(g_contrib[s], sn[s], "lp");
    }
    l3->Draw();

    // Panel 4: Text
    c->cd(4);
    TPaveText *txt = new TPaveText(0.05, 0.05, 0.95, 0.95, "NDC");
    txt->SetTextFont(42); txt->SetTextSize(0.035);
    txt->AddText("Class 2 = largest grains");
    txt->AddText("Primary Xe beam + heavy fragments");
    txt->AddText("");
    txt->AddText("Fraction drops at Bragg peak");
    txt->AddText("Area peaks before stop (Bethe-Bloch max)");
    txt->AddText("Weighted density = frac #times total density");
    txt->Draw();

    c->SaveAs(base + "/analysis/class2_analysis.png");
    printf("Saved class2_analysis.png\n");
}

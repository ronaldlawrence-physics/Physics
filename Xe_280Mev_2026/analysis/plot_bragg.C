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

void plot_bragg() {
    gStyle->SetOptStat(0);
    gStyle->SetPadGridX(1);
    gStyle->SetPadGridY(1);

    TString base = "/home/ronald/JINR/LHEP/Data/Xe_280Mev_2026";

    // ================ READ BRAGG DATA ================
    std::ifstream fin((base + "/analysis/bragg_root.dat").Data());
    if (!fin.is_open()) { printf("ERROR: cannot open bragg_root.dat\n"); return; }

    std::vector<Float_t> d[3], td[3], tdpc[3], ed[3], aa[3];
    Int_t ncyc[3] = {1, 5, 10};
    std::string line, sname;

    std::getline(fin, line); // skip header
    while (std::getline(fin, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::stringstream ss(line);
        std::string sn, pn;
        Float_t dep, tden, tdpcv, edep, aarea;
        Int_t cyc;
        ss >> sn >> pn >> dep >> cyc >> tden >> tdpcv >> edep >> aarea;
        Int_t idx = (sn == "Stack_01") ? 0 : (sn == "Stack_02") ? 1 : 2;
        d[idx].push_back(dep);   td[idx].push_back(tden);
        tdpc[idx].push_back(tdpcv);
        ed[idx].push_back(edep); aa[idx].push_back(aarea);
    }
    fin.close();
    printf("Read data: S01=%zu S02=%zu S03=%zu\n", d[0].size(), d[1].size(), d[2].size());

    // Build TGraphs
    TGraph *g_td[3], *g_ed[3], *g_aa[3];
    Int_t col[3] = {kBlue, kRed, kGreen+2};
    Int_t mkr[3] = {20, 21, 22};
    for (Int_t s = 0; s < 3; s++) {
        g_td[s] = new TGraph(d[s].size(), &d[s][0], &td[s][0]);
        g_ed[s] = new TGraph(d[s].size(), &d[s][0], &ed[s][0]);
        g_aa[s] = new TGraph(d[s].size(), &d[s][0], &aa[s][0]);
        g_td[s]->SetLineWidth(2); g_td[s]->SetLineColor(col[s]); g_td[s]->SetMarkerStyle(mkr[s]); g_td[s]->SetMarkerColor(col[s]);
        g_ed[s]->SetLineWidth(2); g_ed[s]->SetLineColor(col[s]); g_ed[s]->SetMarkerStyle(mkr[s]); g_ed[s]->SetMarkerColor(col[s]);
        g_aa[s]->SetLineWidth(2); g_aa[s]->SetLineColor(col[s]); g_aa[s]->SetMarkerStyle(mkr[s]); g_aa[s]->SetMarkerColor(col[s]);
    }

    // ================ CANVAS 1: Overview ================
    TCanvas *c1 = new TCanvas("c1", "Bragg Peak Analysis", 1600, 1200);
    c1->Divide(2, 2);

    // Panel 1: Track Density
    c1->cd(1);
    Float_t max_td = 0;
    for (auto v : td[0]) max_td = TMath::Max(max_td, v);
    for (auto v : td[1]) max_td = TMath::Max(max_td, v);
    for (auto v : td[2]) max_td = TMath::Max(max_td, v);
    TH1F *h1 = gPad->DrawFrame(0, 0, 20.5, max_td*1.15, ";Depth (mm);Track Density (tracks/mm^{2})");
    h1->GetYaxis()->SetTitleOffset(1.3);
    g_td[0]->Draw("CP"); g_td[1]->Draw("CP"); g_td[2]->Draw("CP");
    TLegend *leg1 = new TLegend(0.6, 0.7, 0.88, 0.88);
    leg1->AddEntry(g_td[0], "Stack 01", "lp"); leg1->AddEntry(g_td[1], "Stack 02", "lp"); leg1->AddEntry(g_td[2], "Stack 03", "lp");
    leg1->Draw();

    // Panel 2: Energy Deposition
    c1->cd(2);
    Float_t max_ed_val = 0;
    for (auto v : ed[0]) max_ed_val = TMath::Max(max_ed_val, v);
    for (auto v : ed[1]) max_ed_val = TMath::Max(max_ed_val, v);
    for (auto v : ed[2]) max_ed_val = TMath::Max(max_ed_val, v);
    TH1F *h2 = gPad->DrawFrame(0, 0, 20.5, max_ed_val*1.2, ";Depth (mm);Energy Deposition (fractional area)");
    h2->GetYaxis()->SetTitleOffset(1.3);
    g_ed[0]->Draw("CP"); g_ed[1]->Draw("CP"); g_ed[2]->Draw("CP");
    leg1->DrawClone();

    // Panel 3: Average Grain Area
    c1->cd(3);
    Float_t max_aa_val = 0;
    for (auto v : aa[0]) max_aa_val = TMath::Max(max_aa_val, v);
    for (auto v : aa[1]) max_aa_val = TMath::Max(max_aa_val, v);
    for (auto v : aa[2]) max_aa_val = TMath::Max(max_aa_val, v);
    TH1F *h3 = gPad->DrawFrame(0, 0, 20.5, max_aa_val*1.2, ";Depth (mm);Average Grain Area (#mum^{2})");
    h3->GetYaxis()->SetTitleOffset(1.3);
    g_aa[0]->Draw("CP"); g_aa[1]->Draw("CP"); g_aa[2]->Draw("CP");
    leg1->DrawClone();

    // Panel 4: Text
    c1->cd(4);
    TPaveText *txt = new TPaveText(0.05, 0.05, 0.95, 0.95, "NDC");
    txt->SetTextFont(42); txt->SetTextSize(0.035);
    txt->AddText("Xe-124 @ 280 MeV/nucleon");
    txt->AddText("60#mum emulsion + 2mm glass");
    txt->AddText("Stack 01: 8 plates (1 cycle)");
    txt->AddText("Stack 02: 10 plates (5 cycles)");
    txt->AddText("Stack 03: 10 plates (10 cycles)");
    txt->AddText("");
    txt->AddText("Track density drop = Bragg peak");
    txt->Draw();
    c1->SaveAs(base + "/analysis/bragg_peak_overview.png");
    printf("Saved bragg_peak_overview.png\n");

    // ================ READ CLASS DATA ================
    std::ifstream fcin((base + "/analysis/classes_root.dat").Data());
    if (!fcin.is_open()) { printf("ERROR: cannot open classes_root.dat\n"); return; }

    struct pt { Float_t depth, frac, area_mean; };
    std::map<TString, std::map<Int_t, std::vector<pt>>> class_data;

    std::getline(fcin, line); // skip header
    while (std::getline(fcin, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::stringstream ss(line);
        std::string sn, pn;
        Int_t cls;
        Float_t dep, frac, am;
        ss >> sn >> pn >> dep >> cls >> frac >> am;
        class_data[TString(sn)][cls].push_back({dep, frac, am});
    }
    fcin.close();
    printf("Class data loaded for %zu stacks\n", class_data.size());

    // ================ CANVAS 2: Class Fractions per Stack ================
    TString snames[3] = {"Stack_01", "Stack_02", "Stack_03"};
    Int_t ncols[6] = {kRed, kBlue, kGreen+2, kOrange, kMagenta, kCyan};

    for (Int_t s = 0; s < 3; s++) {
        if (class_data.find(snames[s]) == class_data.end()) continue;
        auto &stk = class_data[snames[s]];

        TCanvas *c2 = new TCanvas(TString::Format("c_s%d", s+1),
                                   snames[s] + " Classes", 1000, 700);
        c2->SetGrid();

        // Collect unique depths
        std::vector<Float_t> depths;
        for (auto &kv : stk) {
            for (auto &p : kv.second) {
                if (std::find(depths.begin(), depths.end(), p.depth) == depths.end())
                    depths.push_back(p.depth);
            }
        }
        std::sort(depths.begin(), depths.end());
        if (depths.empty()) continue;

        TH1F *hf = gPad->DrawFrame(depths.front()-0.5, 0, depths.back()+0.5, 1.15,
            TString::Format(";Depth (mm);Fraction"));
        hf->SetTitle(snames[s] + " Particle Class Fractions");

        TLegend *leg = new TLegend(0.65, 0.65, 0.88, 0.88);
        leg->SetHeader("Class");

        Int_t ci = 0;
        for (auto &kv : stk) {
            Int_t cid = kv.first;
            auto &pts = kv.second;
            std::sort(pts.begin(), pts.end(),
                      [](const pt &a, const pt &b) { return a.depth < b.depth; });

            std::vector<Float_t> xv, yv;
            for (auto &p : pts) { xv.push_back(p.depth); yv.push_back(p.frac); }

            TGraph *g = new TGraph(xv.size(), &xv[0], &yv[0]);
            g->SetLineWidth(2); g->SetLineColor(ncols[ci%6]);
            g->SetMarkerStyle(20+ci); g->SetMarkerColor(ncols[ci%6]);
            g->Draw("CP");
            leg->AddEntry(g, TString::Format("Class %d", cid), "lp");
            ci++;
        }
        leg->Draw();
        c2->SaveAs(base + "/analysis/" + snames[s] + "_particle_classes.png");
        printf("Saved %s_particle_classes.png\n", snames[s].Data());
    }

    // ================ CANVAS 3: dE/dx (Bragg Peak) ================
    TCanvas *c3 = new TCanvas("c3", "dE/dx Bragg Peak", 1400, 500);
    c3->Divide(3, 1);

    for (Int_t s = 0; s < 3; s++) {
        c3->cd(s+1);
        c3->GetPad(s+1)->SetGrid();
        Float_t maxv = 0;
        for (auto v : ed[s]) maxv = TMath::Max(maxv, v);
        if (maxv == 0) continue;

        TH1F *hf = gPad->DrawFrame(0, 0, 20.5, maxv*1.2,
            ";Depth (mm);dE/dx (fractional grain area)");
        hf->SetTitle(snames[s]);

        TGraph *g = new TGraph(d[s].size(), &d[s][0], &ed[s][0]);
        g->SetLineWidth(2); g->SetLineColor(col[s]);
        g->SetMarkerStyle(mkr[s]); g->SetMarkerColor(col[s]);
        g->Draw("CP");

        TPaveText *lt = new TPaveText(0.5, 0.78, 0.88, 0.88, "NDC");
        lt->AddText(snames[s]); lt->SetFillColor(0); lt->Draw();
    }
    c3->SaveAs(base + "/analysis/bragg_dedx.png");
    printf("Saved bragg_dedx.png\n");
    printf("\nAll plots in %s/analysis/\n", base.Data());
}

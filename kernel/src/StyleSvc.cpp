#ifndef STYLESVC_CPP
#define STYLESVC_CPP

#include "StyleSvc.hpp"

// Global scope variables
TStyle *    lhcbStyle;   // general lhcb style
TPaveText * lhcbName;    // standard lhcb text for plot
TText *     lhcbLabel;   // style for Ttext
TLatex *    lhcbLatex;   // style for TLatex;

// define names for colours
Int_t black   = 1;
Int_t red     = 2;
Int_t green   = 3;
Int_t blue    = 4;
Int_t yellow  = 5;
Int_t magenta = 6;
Int_t cyan    = 7;
Int_t purple  = 9;

TStyle * LHCbStyle() {
    cout << WHITE;
    MessageSvc::Line();
    MessageSvc::Print("Setting LHCbStyle for plotting");
    MessageSvc::Line();
    cout << RESET;

    TGaxis::SetMaxDigits(3);
    lhcbStyle = new TStyle("lhcbStyle", "Standard LHCb plots style");

    // use helvetica-bold-r-normal, precision 2 (rotatable)
    Int_t lhcbFont = 132;
    // line thickness
    Double_t lhcbWidth = 3;

    // use plain black on white colors
    lhcbStyle->SetFrameBorderMode(0);
    lhcbStyle->SetCanvasBorderMode(0);
    lhcbStyle->SetPadBorderMode(0);
    lhcbStyle->SetPadColor(0);
    lhcbStyle->SetCanvasColor(0);
    lhcbStyle->SetStatColor(0);
    //  Modified palette
    // int colors[2] = {2,5};
    // lhcbStyle->SetPalette(2,colors);
    //  Dark body radiator palette - interesting
    // lhcbStyle->SetPalette(53);

    //  Default lhcbStyle palette - pretty ugly
    lhcbStyle->SetPalette(1);

    TColor::InitializeColors();
    const Int_t nRGBs        = 5;
    Double_t    stops[nRGBs] = {0.00, 0.34, 0.61, 0.84, 1.00};
    Double_t    red[nRGBs]   = {0.00, 0.00, 0.87, 1.00, 0.51};
    Double_t    green[nRGBs] = {0.00, 0.81, 1.00, 0.20, 0.00};
    Double_t    blue[nRGBs]  = {0.51, 1.00, 0.12, 0.00, 0.00};
    TColor::CreateGradientColorTable(nRGBs, stops, red, green, blue, 255);
    lhcbStyle->SetNumberContours(255);

    // set the paper & margin sizes
    lhcbStyle->SetPaperSize(20, 26);
    lhcbStyle->SetPadTopMargin(0.05);
    lhcbStyle->SetPadRightMargin(0.10);   // increase for colz plots
    lhcbStyle->SetPadBottomMargin(0.16);
    lhcbStyle->SetPadLeftMargin(0.16);

    // use large fonts
    lhcbStyle->SetTextFont(lhcbFont);
    lhcbStyle->SetTextSize(0.08);
    lhcbStyle->SetLabelFont(lhcbFont, "x");
    lhcbStyle->SetLabelFont(lhcbFont, "y");
    lhcbStyle->SetLabelFont(lhcbFont, "z");
    lhcbStyle->SetLabelSize(0.05, "x");
    lhcbStyle->SetLabelSize(0.05, "y");
    lhcbStyle->SetTitleColor(1);
    lhcbStyle->SetTitleStyle(0);
    lhcbStyle->SetTitleFillColor(0);
    lhcbStyle->SetTitleBorderSize(0);
    lhcbStyle->SetTitleOffset(0.85, "y");
    lhcbStyle->SetLabelSize(0.04, "z");
    // lhcbStyle->SetTitleAlign(23);
    // lhcbStyle->SetTextAlign(22,"x");
    // lhcbStyle->SetTextAlign(22,"y");
    // lhcbStyle->SetTextAlign(22,"z");
    // lhcbStyle->SetTitleAlign(22);
    // lhcbStyle->SetTitleAlign(22,"z");

    //Make LHCb font applied everywhere
    lhcbStyle->SetTitleFont(lhcbFont);
    lhcbStyle->SetTitleFont(lhcbFont,"x");
    lhcbStyle->SetTitleFont(lhcbFont,"y");
    lhcbStyle->SetTitleFont(lhcbFont,"z");
    lhcbStyle->SetLegendFont(lhcbFont);

    lhcbStyle->SetTitleSize(0.06, "x");
    lhcbStyle->SetTitleSize(0.06, "y");
    lhcbStyle->SetTitleSize(0.04, "z");

    // use bold lines and markers
    lhcbStyle->SetLineWidth(lhcbWidth);
    lhcbStyle->SetFrameLineWidth(lhcbWidth);
    lhcbStyle->SetHistLineWidth(lhcbWidth);
    lhcbStyle->SetFuncWidth(5);
    lhcbStyle->SetGridWidth(lhcbWidth);
    lhcbStyle->SetLineStyleString(2, "[12 12]");   // postscript dashes
    lhcbStyle->SetMarkerStyle(20);
    lhcbStyle->SetMarkerSize(0.6);

    // label offsets
    lhcbStyle->SetLabelOffset(0.02);

    // by default, do not display histogram decorations:
    lhcbStyle->SetOptStat(0);
    // lhcbStyle->SetOptStat("emr");  // show only nent -e, mean - m, rms -r
    // full opts at http://root.cern.ch/root/html/TStyle.html#TStyle:SetOptStat
    lhcbStyle->SetStatFormat("6.3g");   // specified as c printf options
    lhcbStyle->SetStatFormat("6.3g");   // specified as c printf options

    lhcbStyle->SetOptTitle(1);
    lhcbStyle->SetOptFit(0001);
    // lhcbStyle->SetOptFit(1011); // order is probability, Chi2, errors, parameters

    // look of the statistics box:
    lhcbStyle->SetStatBorderSize(0);
    lhcbStyle->SetStatFont(lhcbFont);
    lhcbStyle->SetStatFontSize(0.05);
    lhcbStyle->SetStatX(0.9);
    lhcbStyle->SetStatY(0.9);
    lhcbStyle->SetStatW(0.25);
    lhcbStyle->SetStatH(0.15);
    // put tick marks on top and RHS of plots
    lhcbStyle->SetPadTickX(1);
    lhcbStyle->SetPadTickY(1);
    // histogram divisions: only 5 in x to avoid label overlaps
    lhcbStyle->SetNdivisions(510, "x");
    lhcbStyle->SetNdivisions(510, "y");

    //  define style for text
    TText * lhcbLabel = new TText();
    lhcbLabel->SetTextFont(lhcbFont);
    lhcbLabel->SetTextColor(1);
    lhcbLabel->SetTextSize(0.04);
    lhcbLabel->SetTextAlign(12);

    // define style of latex text
    TLatex * lhcbLatex = new TLatex();
    lhcbLatex->SetTextFont(lhcbFont);
    lhcbLatex->SetTextColor(1);
    lhcbLatex->SetTextSize(0.04);
    lhcbLatex->SetTextAlign(12);

    //  Correction for PDF
    lhcbStyle->SetLineScalePS(1);

    // set text on 2D plot format numbers
    lhcbStyle->SetPaintTextFormat("4.2f");

    gROOT->SetStyle("lhcbStyle");
    gROOT->ForceStyle();
    return lhcbStyle;
}

TPaveText * PrintLHCb(const TString optLR, const TString optPrelim, const TString optText) {
    //////////////////////////////////////////////////////////////////////////
    // routine to print 'LHCb', 'LHCb Preliminary' on plots
    // options: optLR=L (top left) / R (top right) of plots
    //          optPrelim= Final (LHCb), Prelim (LHCb Preliminary),
    //          Simul (LHCb simulation), Other
    //          optText= text printed if 'Other' specified
    ////////////////////////////////////////////////////////////////////
    TPaveText * lhcbName = nullptr;
    if (optLR.Contains("R")) {
        lhcbName = new TPaveText(0.70 - lhcbStyle->GetPadRightMargin(), 0.75 - lhcbStyle->GetPadTopMargin(), 0.95 - lhcbStyle->GetPadRightMargin(), 0.85 - lhcbStyle->GetPadTopMargin(), "BRNDC");
    } else if (optLR.Contains("L")) {
        lhcbName = new TPaveText(lhcbStyle->GetPadLeftMargin() + 0.05, 0.87 - lhcbStyle->GetPadTopMargin(), lhcbStyle->GetPadLeftMargin() + 0.30, 0.95 - lhcbStyle->GetPadTopMargin(), "BRNDC");
    } else {
        cout << "printLHCb: option unknown" << optLR << endl;
    }

    if (optPrelim.Contains("F")) {
        lhcbName->AddText("LHCb");
        cout << " F "
             << "LHCb Preliminary " << endl;
    } else if (optPrelim.Contains("P")) {
        lhcbName->AddText("#splitline{LHCb}{#scale[1.0]{Preliminary}}");
        cout << " P "
             << "LHCb Preliminary " << endl;
    } else if (optPrelim.Contains("S")) {
        lhcbName->AddText("#splitline{LHCb}{#scale[1.0]{Simulation}}");
        cout << " S "
             << "LHCb Simulation " << endl;
    } else if (optPrelim.Contains("O")) {
        cout << " O " << optText << endl;
        lhcbName->AddText(optText);
    } else {
        cout << "printLHCb: option unknown " << optPrelim << endl;
    }

    if (lhcbName != nullptr) {
        lhcbName->SetFillColor(0);
        lhcbName->SetTextAlign(12);
        lhcbName->SetBorderSize(0);
        // lhcbName->Draw("same");
    }
    return lhcbName;
}

Color_t GetColor(int i) {
    vector< Color_t > _colors = {kBlack, kRed + 1, kBlue + 1, kMagenta + 1, kCyan + 1, kViolet + 1, kGreen + 1, kOrange + 1, kAzure + 1, kYellow + 1};
    if (i >= _colors.size()) {
        MessageSvc::Warning("GetColor", to_string(i), ">", to_string(_colors.size()), "not available");
        return kWhite;
    }
    return _colors[i];
}

#endif

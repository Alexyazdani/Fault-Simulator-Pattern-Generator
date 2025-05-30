import matplotlib.pyplot as plt

def read_values(filename):
    with open(filename, 'r') as f:
        return [float(line.strip()) for line in f if line.strip()]

def plot_curves(files, labels):
    for fname, label in zip(files, labels):
        y_vals = read_values(fname)
        x_vals = list(range(1, len(y_vals) + 1))
        plt.plot(x_vals, y_vals, label=label)

    plt.xlabel("Test volume")
    plt.ylabel("Fault Coverage")
    plt.title("Fault Coverage vs. Test Volume")
    plt.legend()
    plt.grid(True)
    plt.show()

def main():
    input_files = [
        # "build/dalg_report_1908.out",
        "build/podem_report_499_PFS.out",
        # "build/podem_report_1908_rtpgv1.out",
        # "build/podem_report_1908_rtpgv2.out",
        # "build/podem_report_1908_rtpgv3.out",
        # "build/podem_report_1908_rtpgv4.out",
        # "build/podem_report_1908_df_nl.out",
        # "build/podem_report_1908_df_nh.out",
        # "build/podem_report_1908_df_lh.out",
        # "build/podem_report_1908_df_cc.out",
        # "build/podem_report_1908_jf.out",
        # "build/podem_report_1908_fl_rfl.out",
        # "build/podem_report_1908_fl_scoap_ez.out",
        # "build/podem_report_1908_fl_scoap_hd.out",
        # "build/podem_report_1908_tvc.out",
        # "build/podem_report_880_tvc.out",
        "build/podem_report_499_DFS.out",

    ]
    input_labels = [
        # "DALG - c1908.ckt (Baseline)",
        # "PODEM - c1908.ckt (Baseline)",
        # "PODEM - c1908.ckt (RTPG-v1 80FC)",
        # "PODEM - c1908.ckt (RTPG-v2 0.02 dFC)",
        # "PODEM - c1908.ckt (RTPG-v3 0.02 dFC)",
        # "PODEM - c1908.ckt (RTPG-v4 0.1 dFC)",
        # "PODEM - c1908.ckt (DF-NL)",
        # "PODEM - c1908.ckt (DF-NH)",
        # "PODEM - c1908.ckt (DF-LH)",
        # "PODEM - c1908.ckt (DF-CC)",
        # "PODEM - c1908.ckt (JF)",
        # "PODEM - c1908.ckt (FL-RFL)",
        # "PODEM - c1908.ckt (FL-SCOAP-EZ)",
        # "PODEM - c1908.ckt (FL-SCOAP-HD)",
        # "PODEM - c1908.ckt (TVC)",
        # "PODEM - c880.ckt (TVC)"
        # "PODEM - c1908.ckt (PFS)",
        # "PODEM - c1908.ckt (DFS)",
        "PODEM - c6288.ckt (PFS)",
        "PODEM - c6288.ckt (DFS)"

    ]
    plot_curves(input_files, input_labels)

if __name__ == "__main__":
    main()

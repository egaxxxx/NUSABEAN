#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <string>
#include <map>
#include <algorithm>
#include <limits>
#include <cmath>
#include <cctype>
#include <stdexcept>
#include "json/json.h" 
#include "src/jsoncpp.cpp" 

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

const int MAKS_PENGGUNA = 50;
const int MAKS_KOPI = 100;
const int MAKS_RIWAYAT_PEMBELIAN = 500;
const int MAKS_BARIS_TEKS_BUNGKUS = 10;
const int MAKS_ASAL_UNIK = MAKS_KOPI;

struct Pengguna {
    string namaAkun;
    string kataSandiAkun;
    string peranAkun;
};

struct Kopi {
    int idKopi;
    string namaProdukKopi;
    string asalDaerahKopi;
    double stokTonKopi;
    double hargaBeliPerTonKopi;
    double hargaJualPerKgKopi;
    string rasaKopi;
    string deskripsiKopi;
};

struct RiwayatPembelian {
    string waktuTransaksi;
    string namaPelangganTransaksi;
    string jenisKopiTransaksi;
    double totalHargaTransaksi;
    int jumlahKgTransaksi;
};

struct DataPenjualanPerJenis {
    string namaKopiTerjual;
    int totalKgTerjual;
};

const string NAMA_FILE_DATABASE = "NUSABEAN.json";

Kopi g_daftarKopi[MAKS_KOPI];
int g_jumlahKopi = 0;

Pengguna g_daftarPengguna[MAKS_PENGGUNA];
int g_jumlahPengguna = 0;

RiwayatPembelian g_riwayatPembelian[MAKS_RIWAYAT_PEMBELIAN];
int g_jumlahRiwayatPembelian = 0;

Pengguna g_penggunaSaatIni;

string FormatRupiah(double nilai) {
    long long nilaiBulat = static_cast<long long>(round(nilai));
    string hasilStr = to_string(nilaiBulat);
    int panjang = hasilStr.length();
    int counter = 0;
    string hasilFormat = "";
    for (int i = panjang - 1; i >= 0; i--) {
        counter++;
        hasilFormat = hasilStr[i] + hasilFormat;
        if (counter % 3 == 0 && i > 0) {
            hasilFormat = "." + hasilFormat;
        }
    }
    return "Rp " + hasilFormat;
}

void Enter() {
    cout << "Tekan Enter untuk melanjutkan...";
    cout.flush();
    cin.clear(); 
    string dummy;
    getline(cin, dummy);
    if (cin.fail()) { 
        cin.clear(); 
    }
}

void TanganiKesalahan(const string& pesan) {
    cout << "\nKesalahan: " << pesan << endl;
    Enter();
}

void BersihkanInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

bool AmbilInputInteger(const string& prompt, int& nilai_out, const string& pesanErrorTipe = "Input harus berupa angka!", const string& pesanErrorFormat = "Format input tidak valid. Jangan ada karakter tambahan setelah angka.") {
    string line_input;
    cout << prompt;
    if (!getline(cin, line_input)) {
        cin.clear();
        BersihkanInput(); 
        cout << "\nKesalahan: Gagal membaca input." << endl;
        Enter();
        return false;
    }

    stringstream ss(line_input);
    if (ss >> nilai_out) {
        char sisa;
        if (ss >> sisa) {
            cout << "\nKesalahan: " << pesanErrorFormat << endl;
            Enter();
            return false;
        } else {
            return true; 
        }
    } else {
        cout << "\nKesalahan: " << pesanErrorTipe << endl;
        Enter();
        return false;
    }
}

bool AmbilInputDouble(const string& prompt, double& nilai_out, const string& pesanErrorTipe = "Input harus berupa angka desimal!", const string& pesanErrorFormat = "Format input tidak valid. Jangan ada karakter tambahan setelah angka.") {
    double nilai; 
    string line_input;
    cout << prompt;
    if (!getline(cin, line_input)) {
        cin.clear();
        BersihkanInput();
        cout << "\nKesalahan: Gagal membaca input." << endl;
        Enter();
        return false;
    }

    stringstream ss(line_input);
    if (ss >> nilai) { 
        char sisa;
        if (ss >> sisa) { 
            cout << "\nKesalahan: " << pesanErrorFormat << endl;
            Enter();
            return false;
        } else {
            nilai_out = nilai; 
            return true;
        }
    } else { 
        cout << "\nKesalahan: " << pesanErrorTipe << endl;
        Enter();
        return false;
    }
}


string PotongSpasiTepi(const string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

bool ApakahStringAlphaSpasiHubung(const string& str) {
    string teksYangDipangkas = PotongSpasiTepi(str);
    if (teksYangDipangkas.empty()) return false;
    for (int i = 0; i < teksYangDipangkas.length(); ++i) {
        char karakter = teksYangDipangkas[i];
        if (!isalpha(karakter) && karakter != ' ' && karakter != '-') return false;
    }
    return true;
}

void BungkusTeksBaris(const string& teks, size_t lebarKolom, string* larikBarisKeluaran, int& jumlahBaris, int maksBaris) {
    jumlahBaris = 0;
    if (teks.empty()) {
        if (jumlahBaris < maksBaris) {
            larikBarisKeluaran[jumlahBaris++] = "";
        }
        return;
    }
    if (lebarKolom < 1) lebarKolom = 1;

    istringstream aliranKata(teks);
    string kata;
    string barisSaatIni;

    bool tidakAdaSpasi = (teks.find_first_of(" \t\n\r") == string::npos);

    if (tidakAdaSpasi && teks.length() > lebarKolom) {
        for (size_t i = 0; i < teks.length(); i += lebarKolom) {
            if (jumlahBaris < maksBaris) {
                larikBarisKeluaran[jumlahBaris++] = teks.substr(i, lebarKolom);
            } else {
                break;
            }
        }
        if (jumlahBaris == 0 && !teks.empty() && jumlahBaris < maksBaris) {
             larikBarisKeluaran[jumlahBaris++] = teks;
        }
        if (jumlahBaris == 0 && jumlahBaris < maksBaris) {
             larikBarisKeluaran[jumlahBaris++] = "";
        }
        return;
    }


    while (aliranKata >> kata) {
        if (jumlahBaris >= maksBaris) break;

        if (kata.length() > lebarKolom) {
            if (!barisSaatIni.empty()) {
                if (jumlahBaris < maksBaris) larikBarisKeluaran[jumlahBaris++] = barisSaatIni;
                barisSaatIni.clear();
                if (jumlahBaris >= maksBaris) break;
            }
            for (size_t i = 0; i < kata.length(); i += lebarKolom) {
                if (jumlahBaris < maksBaris) larikBarisKeluaran[jumlahBaris++] = kata.substr(i, lebarKolom);
                else break;
            }
        } else {
            if (barisSaatIni.empty()) {
                barisSaatIni = kata;
            } else if (barisSaatIni.length() + 1 + kata.length() <= lebarKolom) {
                barisSaatIni += " " + kata;
            } else {
                if (jumlahBaris < maksBaris) larikBarisKeluaran[jumlahBaris++] = barisSaatIni;
                barisSaatIni = kata;
            }
        }
        if (jumlahBaris >= maksBaris && !barisSaatIni.empty() && aliranKata.peek() != EOF) {
             break;
        }
    }
    if (!barisSaatIni.empty() && jumlahBaris < maksBaris) {
        larikBarisKeluaran[jumlahBaris++] = barisSaatIni;
    }
    if (jumlahBaris == 0 && !teks.empty() && jumlahBaris < maksBaris) {
        larikBarisKeluaran[jumlahBaris++] = teks.substr(0, lebarKolom);
    }
     if (jumlahBaris == 0 && jumlahBaris < maksBaris) {
        larikBarisKeluaran[jumlahBaris++] = "";
    }
}


void SimpanSemuaDataKeJson() {
    Json::Value data_db_json;
    Json::Value larik_pengguna_json(Json::arrayValue);

    for (int i = 0; i < g_jumlahPengguna; ++i) {
        const Pengguna& pengguna = g_daftarPengguna[i];
        Json::Value obj_pengguna_json;
        obj_pengguna_json["namaPengguna"] = pengguna.namaAkun;
        obj_pengguna_json["kataSandi"] = pengguna.kataSandiAkun;
        obj_pengguna_json["peran"] = pengguna.peranAkun;
        larik_pengguna_json.append(obj_pengguna_json);
    }
    data_db_json["daftarPengguna"] = larik_pengguna_json;

    Json::Value larik_kopi_json(Json::arrayValue);
    for (int i = 0; i < g_jumlahKopi; ++i) {
        const Kopi& kopi = g_daftarKopi[i];
        Json::Value obj_kopi_json;
        obj_kopi_json["id"] = kopi.idKopi;
        obj_kopi_json["nama"] = kopi.namaProdukKopi;
        obj_kopi_json["asal"] = kopi.asalDaerahKopi;
        obj_kopi_json["stok"] = kopi.stokTonKopi;
        obj_kopi_json["hargaBeliTon"] = kopi.hargaBeliPerTonKopi;
        obj_kopi_json["hargaJualKg"] = kopi.hargaJualPerKgKopi;
        obj_kopi_json["rasa"] = kopi.rasaKopi;
        obj_kopi_json["deskripsi"] = kopi.deskripsiKopi;
        larik_kopi_json.append(obj_kopi_json);
    }
    data_db_json["daftarKopi"] = larik_kopi_json;

    Json::Value larik_riwayat_json(Json::arrayValue);
    for (int i = 0; i < g_jumlahRiwayatPembelian; ++i) {
        const RiwayatPembelian& pembelian = g_riwayatPembelian[i];
        Json::Value obj_pembelian_json;
        obj_pembelian_json["waktu"] = pembelian.waktuTransaksi;
        obj_pembelian_json["pembeli"] = pembelian.namaPelangganTransaksi;
        obj_pembelian_json["namaKopi"] = pembelian.jenisKopiTransaksi;
        obj_pembelian_json["totalHarga"] = pembelian.totalHargaTransaksi;
        obj_pembelian_json["jumlahKg"] = pembelian.jumlahKgTransaksi;
        larik_riwayat_json.append(obj_pembelian_json);
    }
    data_db_json["riwayatPembelian"] = larik_riwayat_json;

    Json::StreamWriterBuilder pembangunPenulis;
    pembangunPenulis["indentation"] = "    ";

    ofstream file_keluaran(NAMA_FILE_DATABASE);
    if (file_keluaran.is_open()) {
        string stringJson = Json::writeString(pembangunPenulis, data_db_json);
        file_keluaran << stringJson;
        file_keluaran.close();
    } else {
        cerr << "KRITIS: Tidak dapat menyimpan data ke " << NAMA_FILE_DATABASE << endl;
        Enter();
    }
}

void MuatSemuaDataDariJson() {
    Json::Value data_db_json;
    Json::CharReaderBuilder readerBuilder; 
    string errs;
    ifstream file_masukan(NAMA_FILE_DATABASE);
    bool file_baru_dibuat = false;

    if (file_masukan.is_open()) {
        if (file_masukan.peek() == ifstream::traits_type::eof()) {
            file_baru_dibuat = true;
        } else {
            // Using CharReader for parsing
            unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
            if (!Json::parseFromStream(readerBuilder, file_masukan, &data_db_json, &errs)) {
                cerr << "Kesalahan parsing JSON: " << errs
                     << ". Membuat file database baru." << endl;
                file_baru_dibuat = true;
            }
        }
        file_masukan.close();
    } else {
        file_baru_dibuat = true;
    }

    g_jumlahPengguna = 0;
    if (data_db_json.isMember("daftarPengguna") && data_db_json["daftarPengguna"].isArray()) {
        const Json::Value& larik_pengguna_json = data_db_json["daftarPengguna"];
        for (unsigned int i = 0; i < larik_pengguna_json.size(); ++i) {
            if (g_jumlahPengguna >= MAKS_PENGGUNA) {
                cerr << "Peringatan: Jumlah pengguna maksimum (" << MAKS_PENGGUNA << ") tercapai. Data pengguna selanjutnya tidak dimuat." << endl;
                break;
            }
            const Json::Value& pengguna_json = larik_pengguna_json[i];
            try {
                if (pengguna_json.isMember("namaPengguna") && pengguna_json["namaPengguna"].isString() &&
                    pengguna_json.isMember("kataSandi") && pengguna_json["kataSandi"].isString() &&
                    pengguna_json.isMember("peran") && pengguna_json["peran"].isString()) {

                    g_daftarPengguna[g_jumlahPengguna].namaAkun = pengguna_json["namaPengguna"].asString();
                    g_daftarPengguna[g_jumlahPengguna].kataSandiAkun = pengguna_json["kataSandi"].asString();
                    g_daftarPengguna[g_jumlahPengguna].peranAkun = pengguna_json["peran"].asString();
                    g_jumlahPengguna++;
                } else {
                    cerr << "Data pengguna di JSON tidak lengkap atau tipe salah." << endl;
                }
            } catch (const Json::Exception& e) {
                cerr << "Kesalahan data pengguna di JSON: " << e.what() << endl;
            }
        }
    }

    if (g_jumlahPengguna == 0 && file_baru_dibuat) {
        cout << "Tidak ada pengguna ditemukan atau file baru. Membuat akun admin default (Admin/123)..." << endl;
        if (g_jumlahPengguna < MAKS_PENGGUNA) {
            g_daftarPengguna[g_jumlahPengguna] = {"Admin", "123", "Admin"};
            g_jumlahPengguna++;
        } else {
             cerr << "Tidak dapat membuat admin default, kapasitas pengguna penuh." << endl;
        }
    }

    g_jumlahKopi = 0;
    if (data_db_json.isMember("daftarKopi") && data_db_json["daftarKopi"].isArray()) {
        const Json::Value& larik_kopi_json = data_db_json["daftarKopi"];
        for (unsigned int i = 0; i < larik_kopi_json.size(); ++i) {
            if (g_jumlahKopi >= MAKS_KOPI) {
                cerr << "Peringatan: Jumlah kopi maksimum (" << MAKS_KOPI << ") tercapai. Data kopi selanjutnya tidak dimuat." << endl;
                break;
            }
            const Json::Value& kopi_json = larik_kopi_json[i];
            try {
                if (kopi_json.isMember("id") && kopi_json["id"].isInt() &&
                    kopi_json.isMember("nama") && kopi_json["nama"].isString() &&
                    kopi_json.isMember("asal") && kopi_json["asal"].isString() &&
                    kopi_json.isMember("stok") && kopi_json["stok"].isNumeric() &&
                    kopi_json.isMember("hargaBeliTon") && kopi_json["hargaBeliTon"].isNumeric() &&
                    kopi_json.isMember("hargaJualKg") && kopi_json["hargaJualKg"].isNumeric()) {

                    g_daftarKopi[g_jumlahKopi].idKopi = kopi_json["id"].asInt();
                    g_daftarKopi[g_jumlahKopi].namaProdukKopi = kopi_json["nama"].asString();
                    g_daftarKopi[g_jumlahKopi].asalDaerahKopi = kopi_json["asal"].asString();
                    g_daftarKopi[g_jumlahKopi].stokTonKopi = kopi_json["stok"].asDouble();
                    g_daftarKopi[g_jumlahKopi].hargaBeliPerTonKopi = kopi_json["hargaBeliTon"].asDouble();
                    g_daftarKopi[g_jumlahKopi].hargaJualPerKgKopi = kopi_json["hargaJualKg"].asDouble();
                    g_daftarKopi[g_jumlahKopi].rasaKopi = (kopi_json.isMember("rasa") && kopi_json["rasa"].isString()) ? kopi_json["rasa"].asString() : "";
                    g_daftarKopi[g_jumlahKopi].deskripsiKopi = (kopi_json.isMember("deskripsi") && kopi_json["deskripsi"].isString()) ? kopi_json["deskripsi"].asString() : "";
                    g_jumlahKopi++;
                } else {
                     cerr << "Data kopi di JSON tidak lengkap atau tipe salah (ID:" << (kopi_json.isMember("id")? kopi_json["id"].asString() : "N/A") << ")" << endl;
                }
            } catch (const Json::Exception& e) {
                cerr << "Kesalahan data kopi di JSON: " << e.what() << endl;
            }
        }
    }

    g_jumlahRiwayatPembelian = 0;
    if (data_db_json.isMember("riwayatPembelian") && data_db_json["riwayatPembelian"].isArray()) {
        const Json::Value& larik_riwayat_json = data_db_json["riwayatPembelian"];
        for (unsigned int i = 0; i < larik_riwayat_json.size(); ++i) {
             if (g_jumlahRiwayatPembelian >= MAKS_RIWAYAT_PEMBELIAN) {
                cerr << "Peringatan: Jumlah riwayat pembelian maksimum (" << MAKS_RIWAYAT_PEMBELIAN << ") tercapai. Data riwayat selanjutnya tidak dimuat." << endl;
                break;
            }
            const Json::Value& pembelian_json = larik_riwayat_json[i];
            try {
                 if (pembelian_json.isMember("waktu") && pembelian_json["waktu"].isString() &&
                    pembelian_json.isMember("pembeli") && pembelian_json["pembeli"].isString() &&
                    pembelian_json.isMember("namaKopi") && pembelian_json["namaKopi"].isString() &&
                    pembelian_json.isMember("totalHarga") && pembelian_json["totalHarga"].isNumeric() &&
                    pembelian_json.isMember("jumlahKg") && pembelian_json["jumlahKg"].isInt()) {

                    g_riwayatPembelian[g_jumlahRiwayatPembelian].waktuTransaksi = pembelian_json["waktu"].asString();
                    g_riwayatPembelian[g_jumlahRiwayatPembelian].namaPelangganTransaksi = pembelian_json["pembeli"].asString();
                    g_riwayatPembelian[g_jumlahRiwayatPembelian].jenisKopiTransaksi = pembelian_json["namaKopi"].asString();
                    g_riwayatPembelian[g_jumlahRiwayatPembelian].totalHargaTransaksi = pembelian_json["totalHarga"].asDouble();
                    g_riwayatPembelian[g_jumlahRiwayatPembelian].jumlahKgTransaksi = pembelian_json["jumlahKg"].asInt();
                    g_jumlahRiwayatPembelian++;
                } else {
                    cerr << "Data riwayat pembelian di JSON tidak lengkap atau tipe salah." << endl;
                }
            } catch (const Json::Exception& e) {
                cerr << "Kesalahan data riwayat pembelian di JSON: " << e.what() << endl;
            }
        }
    }

    if (file_baru_dibuat) {
        SimpanSemuaDataKeJson();
    }
}

string DapatkanCapWaktuSaatIni() {
    time_t now = time(0);
    tm* waktuLokal = localtime(&now);
    char bufferWaktu[80];
    strftime(bufferWaktu, 80, "%Y-%m-%d %H:%M:%S", waktuLokal);
    return string(bufferWaktu);
}

void MenuAdmin() {
    cout << "\n╔══════════════════════════════════╗\n";
    cout << "║            MENU ADMIN            ║\n";
    cout << "╠══════════════════════════════════╣\n";
    cout << "║ 1. Tambah Jenis Biji Kopi        ║\n";
    cout << "║ 2. Manajemen Data Kopi           ║\n";
    cout << "║ 3. Lihat Riwayat Penjualan       ║\n";
    cout << "║ 4. Lihat Laporan Transaksi       ║\n";
    cout << "║ 5. Hapus Riwayat Penjualan       ║\n";
    cout << "║ 6. Logout                        ║\n";
    cout << "╚══════════════════════════════════╝\n";
}

void MenuPelanggan() {
    cout << "\n╔═══════════════════════════════════════╗\n";
    cout << "║           MENU PELANGGAN              ║\n";
    cout << "╠═══════════════════════════════════════╣\n";
    cout << "║ 1. Membeli Biji Kopi                  ║\n";
    cout << "║ 2. Cari Biji  Kopi berdasarkan Asal   ║\n";
    cout << "║ 3. Lihat Daftar Biji Kopi             ║\n";
    cout << "║ 4. Lihat Biji Kopi Terlaris           ║\n";
    cout << "║ 5. Lihat Riwayat Pembelian            ║\n";
    cout << "║ 6. Logout                             ║\n";
    cout << "╚═══════════════════════════════════════╝\n";
}

void LihatSemuaKopi();
void DetailPenjualan(const DataPenjualanPerJenis larikPenjualanTerurut[], int jumlahData);
void LaporanPenjualanl();


void CariAsalKopi() {
    string daftarAsalUnik[MAKS_ASAL_UNIK];
    int jumlahAsalUnik = 0;

    for (int i = 0; i < g_jumlahKopi; ++i) {
        bool sudahAda = false;
        for (int j = 0; j < jumlahAsalUnik; ++j) {
            if (g_daftarKopi[i].asalDaerahKopi == daftarAsalUnik[j]) {
                sudahAda = true;
                break;
            }
        }
        if (!sudahAda && jumlahAsalUnik < MAKS_ASAL_UNIK) {
            daftarAsalUnik[jumlahAsalUnik++] = g_daftarKopi[i].asalDaerahKopi;
        }
    }

    char karakterPengisiLama = cout.fill();
    cout.fill(' ');

    if (jumlahAsalUnik == 0) {
        cout << "\n┌───────────────────────────────────────────────────────────────┐\n";
        cout << "│                                                               │\n";
        cout << "│                   Tidak ada data kopi tersedia                │\n";
        cout << "│                                                               │\n";
        cout << "└───────────────────────────────────────────────────────────────┘\n\n";
    } else {
        cout << "\n╔═══════════════════════════════════════════════════════════════╗\n";
        cout << "║           Daftar Daerah Asal Kopi yang Tersedia               ║\n";
        cout << "╠═══════════════════════════════════════════════════════════════╣\n";

        for (int i = 0; i < jumlahAsalUnik; ++i) {
            cout << "║ " << setw(2) << (i + 1) << ". " << setw(57) << left << daftarAsalUnik[i].substr(0, 57) << " ║\n";
        }
        cout << "╚═══════════════════════════════════════════════════════════════╝\n\n";
    }
    cout.fill(karakterPengisiLama);

    string searchOrigin;
    cout << "Masukkan daerah asal kopi: ";
    getline(cin, searchOrigin);

    if (searchOrigin.empty() || PotongSpasiTepi(searchOrigin).empty()) {
        cout << "\nPencarian tidak boleh kosong!\n";
        Enter();
        return;
    }
    searchOrigin = PotongSpasiTepi(searchOrigin);

    cout << "\n╔═══════════════════════════════════════════════════════════════╗\n";
    cout << "║           Hasil Pencarian Kopi dari " << left << setw(20) << searchOrigin.substr(0,20) << "      ║\n";
    cout << "╠═══════════════════════════════════════════════════════════════╣\n";

    bool found = false;
    for (int i = 0; i < g_jumlahKopi; ++i) {
        const Kopi& kopi = g_daftarKopi[i];
        string AsalLower = kopi.asalDaerahKopi;
        string searchLower = searchOrigin;

        transform(AsalLower.begin(), AsalLower.end(), AsalLower.begin(), ::tolower);
        transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);


        if (AsalLower.find(searchLower) != string::npos) {
            if (!found) {
                cout << "║" << setw(5) << " ID"
                     << "║" << setw(15) << " Nama"
                     << "║" << setw(15) << " Asal"
                     << "║" << setw(12) << " Harga/Kg"
                     << "║" << setw(12) << " Rasa" << "║\n";
                cout << "╠═══════════════════════════════════════════════════════════════╣\n";
            }
            cout << "║" << setw(5) << ("K-" + to_string(kopi.idKopi))
                 << "║" << setw(15) << kopi.namaProdukKopi.substr(0,14)
                 << "║" << setw(15) << kopi.asalDaerahKopi.substr(0,14)
                 << "║" << setw(12) << FormatRupiah(kopi.hargaJualPerKgKopi).substr(0,11)
                 << "║" << setw(12) << kopi.rasaKopi.substr(0,11) << "║\n";
            found = true;
        }
    }

    if (!found) {
        system("cls");
        cout << "\n╔════════════════════════════════════════════════════════════════╗\n";
        cout << "║                                                                ║\n";
        cout << "║      Tidak ditemukan kopi berdasarkan Asal yang kamu cari      ║\n";
        cout << "║                                                                ║\n";
        cout << "║                                                                ║\n";
    }
    cout << "╚════════════════════════════════════════════════════════════════╝\n";
    cout.fill(karakterPengisiLama);
    Enter();
}


int CariIndeksKopiDenganID(int idDicari) {
    for (int i = 0; i < g_jumlahKopi; ++i) {
        if (g_daftarKopi[i].idKopi == idDicari) return i;
    }
    return -1;
}

int ParseIdKopiDariString(const string& inputStr, bool& berhasil) {
    berhasil = false;
    string str = PotongSpasiTepi(inputStr);
    if (str.empty()) {
        cout << "Input ID tidak boleh kosong." << endl;
        return -1;
    }

    int idHasil = -1;
    try {
        if ((str[0] == 'K' || str[0] == 'k') && str.length() > 1 && str[1] == '-') {
            if (str.length() <= 2) {
                 cout << "Format K-XX tidak valid, nomor hilang." << endl;
                 return -1;
            }
            string bagianAngka = str.substr(2);
            if (bagianAngka.empty() || bagianAngka.find_first_not_of("0123456789") != string::npos) {
                cout << "Bagian nomor setelah K- harus angka." << endl;
                return -1;
            }
            idHasil = stoi(bagianAngka);
        } else {
             if (str.find_first_not_of("0123456789") != string::npos) {
                cout << "Format ID tidak valid, harus angka atau K-XX." << endl;
                return -1;
            }
            idHasil = stoi(str);
        }
        berhasil = true;
        return idHasil;
    } catch (const std::out_of_range&) {
        cout << "Nomor ID di luar jangkauan." << endl;
        return -1;
    } catch (const std::invalid_argument&) {
        cout << "Format ID tidak valid." << endl;
        return -1;
    }
}

void MenuManajemenData() {
    int pilihanMenu = 0;
    string masukanIdString;
    int idUntukDikelola;
    int indeksKopi;

    do {
            #ifdef _WIN32
                system("cls");
            #else
                system("clear");
            #endif
            LihatSemuaKopi(); 
            if (g_jumlahKopi == 0 && pilihanMenu !=9) { 
                cout << "Tidak ada data kopi untuk dikelola." << endl;
                Enter();
                return; 
            }

            cout << "\n╔══════════════════════════════════════════════╗\n";
            cout << "║           Menu Manajemen Data                ║\n";
            cout << "╠══════════════════════════════════════════════╣\n";
            cout << "║ 1. Update Jenis Biji Kopi                    ║\n";
            cout << "║ 2. Update Asal Biji Kopi                     ║\n";
            cout << "║ 3. Update Stok                               ║\n";
            cout << "║ 4. Update Harga Beli                         ║\n";
            cout << "║ 5. Update Harga Jual                         ║\n";
            cout << "║ 6. Update Rasa/Karakteristik                 ║\n";
            cout << "║ 7. Update Deskripsi Produk                   ║\n";
            cout << "║ 8. Hapus Data                                ║\n";
            cout << "║ 9. Kembali ke Menu Admin                     ║\n";
            cout << "╚══════════════════════════════════════════════╝\n";

            if (!AmbilInputInteger("Pilih menu: ", pilihanMenu)) {
                continue;
            }
            
            if (pilihanMenu == 9) break; 

            if (g_jumlahKopi == 0 && pilihanMenu >=1 && pilihanMenu <=8) {
                cout << "Tidak ada data kopi untuk dikelola." << endl;
                Enter();
                continue;
            }


            if (pilihanMenu >= 1 && pilihanMenu <= 8) {
                cout << "Masukkan No. Kopi yang akan dikelola (contoh: K-1 atau 1): ";
                getline(cin, masukanIdString);
                bool parseBerhasil;
                idUntukDikelola = ParseIdKopiDariString(masukanIdString, parseBerhasil);
                if (!parseBerhasil) {
                    Enter(); 
                    continue; 
                }
                indeksKopi = CariIndeksKopiDenganID(idUntukDikelola);
                if (indeksKopi == -1) {
                    TanganiKesalahan("No. Kopi tidak ditemukan!");
                    continue; 
                }
            }

            char karakterPengisiLama = cout.fill();
            cout.fill(' ');

            switch(pilihanMenu) {
                case 1: {
                    string namaBaru;
                    do {
                        cout << "Masukkan nama/jenis biji kopi baru (maks. 30 karakter): ";
                        getline(cin, namaBaru);
                        namaBaru = PotongSpasiTepi(namaBaru);
                        if (namaBaru.length() > 30) { TanganiKesalahan("Nama kopi tidak boleh lebih dari 30 karakter!"); continue; }
                        if (namaBaru.empty()) { TanganiKesalahan("Nama kopi tidak boleh kosong!"); continue;}
                        if (!ApakahStringAlphaSpasiHubung(namaBaru)) { TanganiKesalahan("Nama hanya boleh huruf, spasi, dan tanda hubung!"); }
                    } while (namaBaru.empty() || namaBaru.length() > 30 || !ApakahStringAlphaSpasiHubung(namaBaru));
                    g_daftarKopi[indeksKopi].namaProdukKopi = namaBaru;
                    SimpanSemuaDataKeJson();
                    cout << "Nama/jenis biji kopi berhasil diupdate!\n"; Enter();
                    break;
                }
                case 2: {
                    string asalBaru;
                     do {
                        cout << "Masukkan asal biji kopi baru (maks. 35 karakter): ";
                        getline(cin, asalBaru);
                        asalBaru = PotongSpasiTepi(asalBaru);
                        if (asalBaru.empty()) { TanganiKesalahan("Asal tidak boleh kosong!"); continue;}
                        if (asalBaru.length() > 35) { TanganiKesalahan("Asal biji kopi tidak boleh lebih dari 35 karakter!"); continue; }
                        if (!ApakahStringAlphaSpasiHubung(asalBaru)) { TanganiKesalahan("Asal hanya boleh huruf, spasi, dan tanda hubung!");}
                    } while (asalBaru.empty() || asalBaru.length() > 35 || !ApakahStringAlphaSpasiHubung(asalBaru));
                    g_daftarKopi[indeksKopi].asalDaerahKopi = asalBaru;
                    SimpanSemuaDataKeJson();
                    cout << "Asal biji kopi berhasil diupdate!\n"; Enter();
                    break;
                }
                case 3: {
                    double stokBaru = 0.0;
                    string stokInputStr;
                    bool stokValid = false;
                    while (!stokValid) { 
                        cout << "Masukkan stok baru (dalam ton, input maks. 10 karakter): ";
                        if (!getline(cin, stokInputStr)) {
                            TanganiKesalahan("Gagal membaca input stok.");
                            cin.clear(); BersihkanInput();
                            continue; 
                        }
                        stokInputStr = PotongSpasiTepi(stokInputStr);

                        if (stokInputStr.length() > 10) {
                            TanganiKesalahan("Input stok terlalu panjang (maks 10 karakter)!"); continue;
                        }
                        if (stokInputStr.empty()) {
                            TanganiKesalahan("Input stok tidak boleh kosong!"); continue;
                        }

                        stringstream ss_stok(stokInputStr);
                        if (ss_stok >> stokBaru) {
                            char sisa_stok;
                            if (ss_stok >> sisa_stok) {
                                TanganiKesalahan("Format input stok tidak valid. Jangan ada karakter tambahan setelah angka.");
                            } else {
                                if (stokBaru < 0) {
                                    TanganiKesalahan("Stok harus berupa angka >= 0!");
                                } else {
                                    stokValid = true; 
                                }
                            }
                        } else {
                            TanganiKesalahan("Input stok harus berupa angka desimal!");
                        }
                    }
                    g_daftarKopi[indeksKopi].stokTonKopi = stokBaru;
                    SimpanSemuaDataKeJson();
                    cout << "Stok berhasil diupdate!\n"; Enter();
                    break;
                }
                case 4: { 
                    double hargaBeliBaru;
                    bool inputOk;
                    do {
                        inputOk = AmbilInputDouble("Masukkan harga beli baru (Rp/ton): Rp ", hargaBeliBaru);
                        if (inputOk && hargaBeliBaru <= 0) {
                            TanganiKesalahan("Harga beli harus berupa angka positif!");
                            inputOk = false; 
                        }
                    } while (!inputOk);
                    g_daftarKopi[indeksKopi].hargaBeliPerTonKopi = hargaBeliBaru;
                    SimpanSemuaDataKeJson();
                    cout << "Harga beli berhasil diupdate!\n"; Enter();
                    break;
                }
                case 5: { 
                    double hargaJualBaru;
                    bool inputOk;
                    do {
                        inputOk = AmbilInputDouble("Masukkan harga jual baru (Rp/kg): Rp ", hargaJualBaru);
                        if (inputOk && hargaJualBaru <= 0) {
                            TanganiKesalahan("Harga jual harus berupa angka positif!");
                            inputOk = false; 
                        }
                    } while (!inputOk);
                    g_daftarKopi[indeksKopi].hargaJualPerKgKopi = hargaJualBaru;
                    SimpanSemuaDataKeJson();
                    cout << "Harga jual berhasil diupdate!\n"; Enter();
                    break;
                }
                case 6: {
                    string rasaBaru;
                    do {
                        cout << "Masukkan rasa/karakteristik baru: ";
                        getline(cin, rasaBaru);
                        rasaBaru = PotongSpasiTepi(rasaBaru);
                        if (rasaBaru.empty()) { TanganiKesalahan("Rasa/Karakteristik tidak boleh kosong!"); continue;}
                        if (!ApakahStringAlphaSpasiHubung(rasaBaru)) { TanganiKesalahan("Rasa/Karakteristik hanya boleh huruf, spasi, dan tanda hubung!"); }
                    } while (rasaBaru.empty() || !ApakahStringAlphaSpasiHubung(rasaBaru));
                    g_daftarKopi[indeksKopi].rasaKopi = rasaBaru;
                    SimpanSemuaDataKeJson();
                    cout << "Rasa/Karakteristik kopi berhasil diupdate!\n"; Enter();
                    break;
                }
                 case 7: {
                    string deskripsiBaru;
                    do {
                        cout << "Masukkan deskripsi produk baru (maks. 350 karakter): ";
                        getline(cin, deskripsiBaru);
                        deskripsiBaru = PotongSpasiTepi(deskripsiBaru);
                        if (deskripsiBaru.length() > 350) {
                            TanganiKesalahan("Deskripsi produk terlalu panjang (maks 350 karakter)!");
                        }
                    } while (deskripsiBaru.length() > 350);
                    g_daftarKopi[indeksKopi].deskripsiKopi = deskripsiBaru;
                    SimpanSemuaDataKeJson();
                    cout << "Deskripsi produk kopi berhasil diupdate!\n"; Enter();
                    break;
                }
                case 8: {
                    cout << "Anda yakin ingin menghapus kopi '" << g_daftarKopi[indeksKopi].namaProdukKopi << "' (No.: K-" << g_daftarKopi[indeksKopi].idKopi << ")? (y/n): ";
                    string konfirmasi;
                    getline(cin, konfirmasi);
                    if (konfirmasi == "y" || konfirmasi == "Y") {
                        for (int i = indeksKopi; i < g_jumlahKopi - 1; ++i) {
                            g_daftarKopi[i] = g_daftarKopi[i+1];
                        }
                        g_jumlahKopi--;
                        SimpanSemuaDataKeJson();
                        cout << "Data berhasil dihapus!\n";
                    } else {
                        cout << "Penghapusan dibatalkan.\n";
                    }
                    Enter();
                    break;
                }
                default:
                    if (pilihanMenu != 9) { 
                       TanganiKesalahan("Pilihan tidak valid!");
                    }
                    break;
            }
            cout.fill(karakterPengisiLama);
    } while (pilihanMenu != 9);
}


void LaporanPenjualanl() {
    bool isAdmin = (g_penggunaSaatIni.peranAkun == "Admin");

    if (g_jumlahRiwayatPembelian == 0) {
        cout << "\n╔════════════════════════════════╗\n";
        if (isAdmin) {
            cout << "║  Belum ada transaksi penjualan ║\n";
        } else {
            cout << "║   Belum ada kopi yang terjual  ║\n";
        }
        cout << "╚════════════════════════════════╝\n";
        Enter();
        return;
    }

    char karakterPengisiLama = cout.fill();
    cout.fill(' ');

    double totalPemasukanKeseluruhan = 0;
    double totalModalKeseluruhan = 0;
    map<string, int> petaTotalKopiTerjualKg;

    for (int i = 0; i < g_jumlahRiwayatPembelian; ++i) {
        const RiwayatPembelian& pembelian = g_riwayatPembelian[i];
        petaTotalKopiTerjualKg[pembelian.jenisKopiTransaksi] += pembelian.jumlahKgTransaksi;
        if (isAdmin) {
            totalPemasukanKeseluruhan += pembelian.totalHargaTransaksi;
        }
    }

    if (petaTotalKopiTerjualKg.empty()) {
         cout << "\n╔════════════════════════════════╗\n";
         cout << "║   Belum ada kopi yang terjual  ║\n";
         cout << "╚════════════════════════════════╝\n";
         cout.fill(karakterPengisiLama);
         Enter();
         return;
    }

    if (isAdmin) {
        for (auto const& [namaJenisKopi, jumlahTerjualKg] : petaTotalKopiTerjualKg) {
            for (int j = 0; j < g_jumlahKopi; ++j) {
                if (g_daftarKopi[j].namaProdukKopi == namaJenisKopi) {
                    totalModalKeseluruhan += (static_cast<double>(jumlahTerjualKg) / 1000.0) * g_daftarKopi[j].hargaBeliPerTonKopi;
                    break;
                }
            }
        }
        double labaBersihKeseluruhan = totalPemasukanKeseluruhan - totalModalKeseluruhan;

        cout << "\n╔═══════════════════════════════════════════════╗\n";
        cout << "║            LAPORAN TRANSAKSI                  ║\n";
        cout << "╠═══════════════════════════════════════════════╣\n";
        cout << fixed << setprecision(2);
        cout << "║ Total Penjualan : " << left << setw(25) << FormatRupiah(totalPemasukanKeseluruhan) << "║\n";
        cout << "║ Total Modal     : " << left << setw(25) << FormatRupiah(totalModalKeseluruhan) << "║\n";
        cout << "║ Laba Bersih     : " << left << setw(25) << FormatRupiah(labaBersihKeseluruhan) << "║\n";
        cout << "╚═══════════════════════════════════════════════╝\n";
        cout << defaultfloat;
    }


    DataPenjualanPerJenis larikPenjualanTerurut[MAKS_KOPI];
    int jumlahDataPenjualanAktual = 0;

    for (auto const& [namaKopi, totalKg] : petaTotalKopiTerjualKg) {
        if (jumlahDataPenjualanAktual < MAKS_KOPI) {
            larikPenjualanTerurut[jumlahDataPenjualanAktual].namaKopiTerjual = namaKopi;
            larikPenjualanTerurut[jumlahDataPenjualanAktual].totalKgTerjual = totalKg;
            jumlahDataPenjualanAktual++;
        } else {
            break;
        }
    }
    for (int i = 0; i < jumlahDataPenjualanAktual - 1; ++i) {
        for (int j = 0; j < jumlahDataPenjualanAktual - i - 1; ++j) {
            if (larikPenjualanTerurut[j].totalKgTerjual < larikPenjualanTerurut[j + 1].totalKgTerjual) {
                DataPenjualanPerJenis temp = larikPenjualanTerurut[j];
                larikPenjualanTerurut[j] = larikPenjualanTerurut[j+1];
                larikPenjualanTerurut[j+1] = temp;
            }
        }
    }

    if(!isAdmin){
        cout << "\n=========== PERINGKAT BIJI KOPI TERLARIS ===========\n";
    }
    DetailPenjualan(larikPenjualanTerurut, jumlahDataPenjualanAktual);

    cout.fill(karakterPengisiLama);
    Enter();
}

void FormatTampilanKopiUniversal(const Kopi daftarKopiDitampilkan[], int jumlah) {
    char karakterPengisiLama = cout.fill();
    cout.fill(' ');

    bool isAdmin = (g_penggunaSaatIni.peranAkun == "Admin");

    const size_t LEBAR_NO = 5;
    const size_t LEBAR_NAMA = 25;
    const size_t LEBAR_ASAL = 20;
    const size_t LEBAR_STOK = 12;
    const size_t LEBAR_DESKRIPSI = 45;
    const size_t LEBAR_HARGA_JUAL = 18;
    const size_t LEBAR_HARGA_BELI_KOLOM = 18;

    size_t current_total_cell_widths = LEBAR_NO + LEBAR_NAMA + LEBAR_ASAL + LEBAR_STOK +
                                       LEBAR_DESKRIPSI + LEBAR_HARGA_JUAL;
    int num_displayed_columns = 6;

    if (isAdmin) {
        current_total_cell_widths += LEBAR_HARGA_BELI_KOLOM;
        num_displayed_columns++;
    }
    size_t LEBAR_TOTAL_TABEL = current_total_cell_widths + num_displayed_columns + 1;


    string garisAtas = "╔", garisTengahHeader = "╠", garisBawah = "╚", pemisahVertikal = "║";
    for (size_t i = 0; i < LEBAR_TOTAL_TABEL - 2; i++) {
        garisAtas += "═"; garisTengahHeader += "═"; garisBawah += "═";
    }
    garisAtas += "╗"; garisTengahHeader += "╣"; garisBawah += "╝";

    cout << "\n" << garisAtas << "\n";
    string judulTabel = "DAFTAR KOPI TERSEDIA";
    size_t lebarKolomJudul = LEBAR_TOTAL_TABEL - 2;
    string judulUntukTampilan = judulTabel;
    if (judulTabel.length() > lebarKolomJudul) {
        judulUntukTampilan = judulTabel.substr(0, lebarKolomJudul);
    }
    size_t totalPadding = lebarKolomJudul - judulUntukTampilan.length();
    size_t paddingKiri = totalPadding / 2;
    size_t paddingKanan = totalPadding - paddingKiri;

    cout << pemisahVertikal
         << string(paddingKiri, ' ') << judulUntukTampilan << string(paddingKanan, ' ')
         << pemisahVertikal << "\n";
    cout << garisTengahHeader << "\n";

    if (jumlah == 0) {
        cout << pemisahVertikal << left << setw(LEBAR_TOTAL_TABEL - 2) << " Belum ada data kopi yang tersedia." << pemisahVertikal << "\n";
        cout << garisBawah << "\n";
        cout.fill(karakterPengisiLama);
        return;
    }

    cout << pemisahVertikal << " " << setw(LEBAR_NO -1) << left << "No."
         << pemisahVertikal << " " << setw(LEBAR_NAMA -1) << left << "Nama Kopi"
         << pemisahVertikal << " " << setw(LEBAR_ASAL -1) << left << "Asal"
         << pemisahVertikal << " " << setw(LEBAR_STOK -1) << left << "Stok(Ton)"
         << pemisahVertikal << " " << setw(LEBAR_DESKRIPSI -1) << left << "Deskripsi"
         << pemisahVertikal << " " << setw(LEBAR_HARGA_JUAL-1) << left << "Harga Jual/Kg";
    if (isAdmin) {
        cout << pemisahVertikal << " " << setw(LEBAR_HARGA_BELI_KOLOM-1) << left << "Harga Beli/Ton";
    }
    cout << pemisahVertikal << "\n";
    cout << garisTengahHeader << "\n";

    string pemisahBarisData = "╟";
    for (size_t i = 0; i < LEBAR_NO; i++) pemisahBarisData += "─"; pemisahBarisData += "╫";
    for (size_t i = 0; i < LEBAR_NAMA; i++) pemisahBarisData += "─"; pemisahBarisData += "╫";
    for (size_t i = 0; i < LEBAR_ASAL; i++) pemisahBarisData += "─"; pemisahBarisData += "╫";
    for (size_t i = 0; i < LEBAR_STOK; i++) pemisahBarisData += "─"; pemisahBarisData += "╫";
    for (size_t i = 0; i < LEBAR_DESKRIPSI; i++) pemisahBarisData += "─"; pemisahBarisData += "╫";
    for (size_t i = 0; i < LEBAR_HARGA_JUAL; i++) pemisahBarisData += "─";
    if (isAdmin) {
        pemisahBarisData += "╫";
        for (size_t i = 0; i < LEBAR_HARGA_BELI_KOLOM; i++) pemisahBarisData += "─";
    }
    pemisahBarisData += "╢";


    string larikNama[MAKS_BARIS_TEKS_BUNGKUS], larikAsal[MAKS_BARIS_TEKS_BUNGKUS],
           larikStok[MAKS_BARIS_TEKS_BUNGKUS], larikDeskripsi[MAKS_BARIS_TEKS_BUNGKUS];
    int jumlahBarisNama, jumlahBarisAsal, jumlahBarisStok, jumlahBarisDeskripsi;

    for (int i = 0; i < jumlah; i++) {
        const Kopi& kopi = daftarKopiDitampilkan[i];

        stringstream ss_no; ss_no << kopi.idKopi;

        BungkusTeksBaris(kopi.namaProdukKopi, LEBAR_NAMA - 2, larikNama, jumlahBarisNama, MAKS_BARIS_TEKS_BUNGKUS);
        BungkusTeksBaris(kopi.asalDaerahKopi, LEBAR_ASAL - 2, larikAsal, jumlahBarisAsal, MAKS_BARIS_TEKS_BUNGKUS);

        stringstream ss_stok_ton_val;
        if (abs(kopi.stokTonKopi - static_cast<int>(kopi.stokTonKopi)) < 0.001 && kopi.stokTonKopi < 1e9) { // Avoid scientific for large whole numbers
            ss_stok_ton_val << static_cast<long long>(kopi.stokTonKopi);
        } else {
            ss_stok_ton_val << fixed << setprecision(2) << kopi.stokTonKopi;
        }
        string strStokTon = ss_stok_ton_val.str();
        BungkusTeksBaris(strStokTon, LEBAR_STOK - 2, larikStok, jumlahBarisStok, MAKS_BARIS_TEKS_BUNGKUS);

        BungkusTeksBaris(kopi.deskripsiKopi, LEBAR_DESKRIPSI - 2, larikDeskripsi, jumlahBarisDeskripsi, MAKS_BARIS_TEKS_BUNGKUS);

        string strHargaJual = FormatRupiah(kopi.hargaJualPerKgKopi);

        string strHargaBeli = "";
        if (isAdmin) {
            strHargaBeli = FormatRupiah(kopi.hargaBeliPerTonKopi);
        }

        size_t maksBarisUntukItemIni = 1;
        if ((size_t)jumlahBarisNama > maksBarisUntukItemIni) maksBarisUntukItemIni = jumlahBarisNama;
        if ((size_t)jumlahBarisAsal > maksBarisUntukItemIni) maksBarisUntukItemIni = jumlahBarisAsal;
        if ((size_t)jumlahBarisStok > maksBarisUntukItemIni) maksBarisUntukItemIni = jumlahBarisStok;
        if ((size_t)jumlahBarisDeskripsi > maksBarisUntukItemIni) maksBarisUntukItemIni = jumlahBarisDeskripsi;


        for (size_t baris = 0; baris < maksBarisUntukItemIni; baris++) {
            cout << pemisahVertikal << " " << setw(LEBAR_NO - 1) << left << (baris == 0 ? "K-" + ss_no.str() : "")
                 << pemisahVertikal << " " << setw(LEBAR_NAMA - 1) << left << (baris < (size_t)jumlahBarisNama ? larikNama[baris] : "")
                 << pemisahVertikal << " " << setw(LEBAR_ASAL - 1) << left << (baris < (size_t)jumlahBarisAsal ? larikAsal[baris] : "")
                 << pemisahVertikal << " " << setw(LEBAR_STOK - 1) << left << (baris < (size_t)jumlahBarisStok ? larikStok[baris] : "")
                 << pemisahVertikal << " " << setw(LEBAR_DESKRIPSI - 1) << left << (baris < (size_t)jumlahBarisDeskripsi ? larikDeskripsi[baris] : "")
                 << pemisahVertikal << " " << setw(LEBAR_HARGA_JUAL - 1) << left << (baris == 0 ? strHargaJual : "");
            if (isAdmin) {
                cout << pemisahVertikal << " " << setw(LEBAR_HARGA_BELI_KOLOM - 1) << left << (baris == 0 ? strHargaBeli : "");
            }
            cout << pemisahVertikal << "\n";
        }
        std::cout << defaultfloat; // Reset float formatting

        if (i < jumlah - 1) {
            cout << pemisahBarisData << "\n";
        }
    }
    cout << garisBawah << "\n";
    cout.fill(karakterPengisiLama);
}
void LihatSemuaKopi() {
        if (g_jumlahKopi == 0) {
            char karakterPengisiLama = cout.fill();
            cout.fill(' ');
            cout << "Belum ada data kopi tersimpan.\n";
            cout.fill(karakterPengisiLama);
            Enter(); 
            return;
        }
        FormatTampilanKopiUniversal(g_daftarKopi, g_jumlahKopi);
}

void TambahDataKopiBaru() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
    LihatSemuaKopi();

    if (g_jumlahKopi >= MAKS_KOPI) {
        cout << "Kapasitas data kopi penuh. Tidak dapat menambah data baru.\n";
        Enter();
        return;
    }

    cout << "\n=== Masukkan Data Kopi Baru ===\n";
    Kopi kopiBaru;

    if (g_jumlahKopi == 0) {
        kopiBaru.idKopi = 1;
    } else {
        int maxId = 0;
        for (int i = 0; i < g_jumlahKopi; ++i) {
            if (g_daftarKopi[i].idKopi > maxId) {
                maxId = g_daftarKopi[i].idKopi;
            }
        }
        kopiBaru.idKopi = maxId + 1;
    }

    char original_fill = cout.fill();
    cout << "No. Kopi Baru (otomatis): K-" << kopiBaru.idKopi << endl;
    cout << std::setfill(' ');

    string tempNama;
    bool isDuplicate;
    do {
        isDuplicate = false;
        cout << "Nama Biji Kopi (maks. 30 karakter, huruf, spasi, -): ";
        getline(cin, tempNama);
        tempNama = PotongSpasiTepi(tempNama);

        if (tempNama.empty()) { TanganiKesalahan("Nama kopi tidak boleh kosong!"); continue; }
        if (tempNama.length() > 30) { TanganiKesalahan("Nama kopi terlalu panjang (maks 30 karakter)!"); continue; }
        if (!ApakahStringAlphaSpasiHubung(tempNama)) { TanganiKesalahan("Nama hanya boleh huruf, spasi, dan tanda hubung!"); continue; }

        string tempNamaLower = tempNama;
        transform(tempNamaLower.begin(), tempNamaLower.end(), tempNamaLower.begin(), ::tolower);

        for (int i = 0; i < g_jumlahKopi; ++i) {
            string namaAda = g_daftarKopi[i].namaProdukKopi;
            namaAda = PotongSpasiTepi(namaAda);
            string namaAdaLower = namaAda;
            transform(namaAdaLower.begin(), namaAdaLower.end(), namaAdaLower.begin(), ::tolower);

            if (namaAdaLower == tempNamaLower) {
                TanganiKesalahan("Kopi dengan nama '" + tempNama + "' sudah ada dalam database!");
                isDuplicate = true;
                break;
            }
        }
    } while (isDuplicate || tempNama.empty() || tempNama.length() > 30 || !ApakahStringAlphaSpasiHubung(tempNama));
    kopiBaru.namaProdukKopi = tempNama;

    string tempAsal;
    do {
        cout << "Asal Daerah (maks. 35 karakter, huruf, spasi, -): ";
        getline(cin, tempAsal);
        tempAsal = PotongSpasiTepi(tempAsal);
        if (tempAsal.empty()) { TanganiKesalahan("Asal daerah tidak boleh kosong!"); continue; }
        if (tempAsal.length() > 35) { TanganiKesalahan("Asal daerah terlalu panjang (maks 35 karakter)!"); continue; }
        if (!ApakahStringAlphaSpasiHubung(tempAsal)) { TanganiKesalahan("Asal hanya boleh huruf, spasi, dan tanda hubung!"); }
    } while (tempAsal.empty() || tempAsal.length() > 35 || !ApakahStringAlphaSpasiHubung(tempAsal));
    kopiBaru.asalDaerahKopi = tempAsal;

    string tempRasa;
    do {
        cout << "Rasa/Karakteristik (huruf, spasi, -): ";
        getline(cin, tempRasa);
        tempRasa = PotongSpasiTepi(tempRasa);
        if (tempRasa.empty()) { TanganiKesalahan("Rasa/karakteristik tidak boleh kosong!"); continue; }
        if (!ApakahStringAlphaSpasiHubung(tempRasa)) { TanganiKesalahan("Rasa hanya boleh huruf, spasi, dan tanda hubung!"); }
    } while (tempRasa.empty() || !ApakahStringAlphaSpasiHubung(tempRasa));
    kopiBaru.rasaKopi = tempRasa;

    string tempDeskripsi;
    do {
        cout << "Deskripsi Produk Tambahan (maks. 350 karakter, opsional): ";
        getline(cin, tempDeskripsi);
        tempDeskripsi = PotongSpasiTepi(tempDeskripsi);
        if (tempDeskripsi.length() > 350) {
            TanganiKesalahan("Deskripsi produk terlalu panjang (maks 350 karakter)!");
        }
    } while (tempDeskripsi.length() > 350);
    kopiBaru.deskripsiKopi = tempDeskripsi;

    string stokInputStr;
    bool stokValid = false;
    while(!stokValid) {
        cout << "Stok (dalam Ton, angka > 0, input maks. 10 karakter): ";
        if (!getline(cin, stokInputStr)) {
             TanganiKesalahan("Gagal membaca input stok.");
             cin.clear(); BersihkanInput(); continue;
        }
        stokInputStr = PotongSpasiTepi(stokInputStr);

        if (stokInputStr.length() > 10) {
            TanganiKesalahan("Input stok terlalu panjang (maks 10 karakter)!"); continue;
        }
        if (stokInputStr.empty()) {
            TanganiKesalahan("Input stok tidak boleh kosong!"); continue;
        }

        stringstream ss_stok_tambah(stokInputStr);
        if (ss_stok_tambah >> kopiBaru.stokTonKopi) {
            char sisa_stok_tambah;
            if (ss_stok_tambah >> sisa_stok_tambah) {
                 TanganiKesalahan("Format input stok tidak valid. Jangan ada karakter tambahan setelah angka.");
            } else {
                 if (kopiBaru.stokTonKopi <= 0) {
                    TanganiKesalahan("Jumlah stok harus lebih dari 0 Ton!");
                } else {
                    stokValid = true;
                }
            }
        } else {
             TanganiKesalahan("Input stok harus berupa angka desimal!");
        }
    }

    bool hargaBeliValid = false;
    do {
        if (!AmbilInputDouble("Harga Beli per Ton (Rp, angka > 0): ", kopiBaru.hargaBeliPerTonKopi)) {
            continue; 
        }
        if (kopiBaru.hargaBeliPerTonKopi <= 0) {
            TanganiKesalahan("Harga beli harus positif!");
        } else {
            hargaBeliValid = true;
        }
    } while (!hargaBeliValid);

    bool hargaJualValid = false;
    do {
        if (!AmbilInputDouble("Harga Jual per Kg (Rp, angka > 0): ", kopiBaru.hargaJualPerKgKopi)) {
            continue; 
        }
        if (kopiBaru.hargaJualPerKgKopi <= 0) {
            TanganiKesalahan("Harga jual harus positif!");
        } else {
            hargaJualValid = true;
        }
    } while (!hargaJualValid);


    g_daftarKopi[g_jumlahKopi] = kopiBaru;
    g_jumlahKopi++;
    SimpanSemuaDataKeJson();

    cout << "\nKopi berhasil ditambahkan!\n";
    cout << "==========================\n";
    cout << "Detail Kopi:\n";
    cout << "No.: K-" << kopiBaru.idKopi << endl;
    cout << std::setfill(' ');

    cout << "Nama: " << kopiBaru.namaProdukKopi << endl;
    cout << "Asal: " << kopiBaru.asalDaerahKopi << endl;
    cout << "Rasa: " << kopiBaru.rasaKopi << endl;

    cout << "Stok: ";
    if (abs(kopiBaru.stokTonKopi - static_cast<int>(kopiBaru.stokTonKopi)) < 0.001) {
        cout << static_cast<int>(kopiBaru.stokTonKopi) << " Ton" << endl;
    } else {
        cout << fixed << setprecision(2) << kopiBaru.stokTonKopi << " Ton" << defaultfloat << endl;
    }

    cout << "Harga Beli/Ton: " << FormatRupiah(kopiBaru.hargaBeliPerTonKopi) << endl;
    cout << "Harga Jual/Kg: " << FormatRupiah(kopiBaru.hargaJualPerKgKopi) << endl;
    cout << "Deskripsi: " << (kopiBaru.deskripsiKopi.empty() ? "-" : kopiBaru.deskripsiKopi) << endl;
    cout << "==========================\n";

    cout.fill(original_fill);
    Enter();
}

void ProsesBeliKopi() {
        if (g_jumlahKopi == 0) {
            TanganiKesalahan("Tidak ada kopi yang tersedia untuk dibeli.");
            return;
        }
        #ifdef _WIN32 
            system("cls");
        #else
            system("clear");
        #endif
        LihatSemuaKopi();

        string masukanIdString;
        int idAkanDibeliParse;
        int indeksKopiAkanDibeli = -1;
        int jumlahKgAkanDibeli;

        char karakterPengisiLama = cout.fill();
        cout.fill(' ');

        cout << "\nMasukkan No. Kopi yang ingin dibeli (contoh: K-1 atau 1): ";
        getline(cin, masukanIdString); 
        bool parseBerhasil;
        idAkanDibeliParse = ParseIdKopiDariString(masukanIdString, parseBerhasil);

        if (!parseBerhasil) {
            Enter();
            cout.fill(karakterPengisiLama);
            return;
        }

        indeksKopiAkanDibeli = CariIndeksKopiDenganID(idAkanDibeliParse);

        if (indeksKopiAkanDibeli == -1) {
            TanganiKesalahan("Kopi dengan No. tersebut tidak ditemukan!");
            cout.fill(karakterPengisiLama);
            return;
        }
        Kopi& kopiTerpilih = g_daftarKopi[indeksKopiAkanDibeli];

        cout << "Kopi dipilih: " << kopiTerpilih.namaProdukKopi << " (Stok: ";
        bool stokTampilDecimal = false;
        if (abs(kopiTerpilih.stokTonKopi - static_cast<int>(kopiTerpilih.stokTonKopi)) < 0.001) {
            cout << static_cast<int>(kopiTerpilih.stokTonKopi);
        } else {
            cout << fixed << setprecision(2) << kopiTerpilih.stokTonKopi;
            stokTampilDecimal = true;
        }
        cout << " Ton)";
        if (stokTampilDecimal) {
            cout << defaultfloat;
        }
        cout << endl;

        // Input quantity using AmbilInputInteger with a loop
        bool qtyValid = false;
        while(!qtyValid) {
            if (!AmbilInputInteger(
                "Masukkan jumlah yang ingin dibeli (kg): ", jumlahKgAkanDibeli,
                "Jumlah pembelian harus berupa angka!",
                "Format jumlah pembelian tidak valid. Jangan ada karakter tambahan setelah angka."
            )) {
                continue; // AmbilInputInteger handled error, re-prompt
            }
            if (jumlahKgAkanDibeli <= 0) {
                TanganiKesalahan("Jumlah pembelian harus berupa angka positif!");
            } else {
                qtyValid = true;
            }
        }


        double stokDibutuhkanTon = static_cast<double>(jumlahKgAkanDibeli) / 1000.0;
        if (kopiTerpilih.stokTonKopi < stokDibutuhkanTon - 0.00001) {
             stringstream ss_stok_ton, ss_stok_kg;
             ss_stok_ton << fixed << setprecision(3) << kopiTerpilih.stokTonKopi;
             ss_stok_kg << fixed << setprecision(0) << round(kopiTerpilih.stokTonKopi * 1000);
             TanganiKesalahan("Stok kopi tidak mencukupi! Tersedia: " + ss_stok_ton.str() + " Ton (" + ss_stok_kg.str() + " Kg).");
             cout.fill(karakterPengisiLama);
             return;
        }

        kopiTerpilih.stokTonKopi -= stokDibutuhkanTon;
        double hargaTotalPembelian = kopiTerpilih.hargaJualPerKgKopi * jumlahKgAkanDibeli;

        if (g_jumlahRiwayatPembelian >= MAKS_RIWAYAT_PEMBELIAN) {
            cout << "Riwayat pembelian penuh, transaksi ini tidak akan disimpan dalam riwayat.\n";
        } else {
            g_riwayatPembelian[g_jumlahRiwayatPembelian].waktuTransaksi = DapatkanCapWaktuSaatIni();
            g_riwayatPembelian[g_jumlahRiwayatPembelian].namaPelangganTransaksi = g_penggunaSaatIni.namaAkun;
            g_riwayatPembelian[g_jumlahRiwayatPembelian].jenisKopiTransaksi = kopiTerpilih.namaProdukKopi;
            g_riwayatPembelian[g_jumlahRiwayatPembelian].totalHargaTransaksi = hargaTotalPembelian;
            g_riwayatPembelian[g_jumlahRiwayatPembelian].jumlahKgTransaksi = jumlahKgAkanDibeli;
            g_jumlahRiwayatPembelian++;
        }
        SimpanSemuaDataKeJson();

        cout << "\nPembelian berhasil!\n";
        cout << "Total harga: " << FormatRupiah(hargaTotalPembelian) << endl;

        cout.fill(karakterPengisiLama);
        Enter();
}

void TampilkanRiwayatPenjualan(bool adminBisaHapus = false) {
    RiwayatPembelian larikRiwayatRelevan[MAKS_RIWAYAT_PEMBELIAN];
    int jumlahRiwayatRelevan = 0;

    if (g_penggunaSaatIni.peranAkun == "Admin") {
        for(int i=0; i < g_jumlahRiwayatPembelian; ++i) {
            if (jumlahRiwayatRelevan < MAKS_RIWAYAT_PEMBELIAN) {
                larikRiwayatRelevan[jumlahRiwayatRelevan++] = g_riwayatPembelian[i];
            } else break;
        }
    } else {
        for(int i=0; i < g_jumlahRiwayatPembelian; ++i) {
            if (g_riwayatPembelian[i].namaPelangganTransaksi == g_penggunaSaatIni.namaAkun) {
                if (jumlahRiwayatRelevan < MAKS_RIWAYAT_PEMBELIAN) {
                     larikRiwayatRelevan[jumlahRiwayatRelevan++] = g_riwayatPembelian[i];
                } else break;
            }
        }
    }

    char karakterPengisiLama = cout.fill();
    cout.fill(' ');

    if (jumlahRiwayatRelevan == 0) {
        cout << "\n╔═══════════════════════════════════╗\n";
        cout << "║  Belum ada riwayat pembelian " << (g_penggunaSaatIni.peranAkun == "Admin" ? "" : "Anda ") << "║\n";
        cout << "╚═══════════════════════════════════╝\n";
        cout.fill(karakterPengisiLama);
        Enter();
        return;
    }

    cout << "\n╔═══════════════════════════════════════════════════════════════════════════════════════════════╗\n";
    cout << "║                                  RIWAYAT PEMBELIAN " << (g_penggunaSaatIni.peranAkun == "Admin" ? "    " : "ANDA") << "                                       ║\n";
    cout << "╠════╦══════════════════════╦═══════════════╦════════════════════╦════════════════╦═════════════╣\n";
    cout << "║ No ║" << left << setw(22) << " Waktu"
         << "║" << setw(15) << " Pembeli"
         << "║" << setw(20) << " Nama Kopi"
         << "║" << setw(16) << "Harga"
         << "║" << setw(13) << "Jumlah (Kg)" << "║\n";
    cout << "╠════╬══════════════════════╬═══════════════╬════════════════════╬════════════════╬═════════════╣\n";

    int nomorUrut = 1;
    for (int i = 0; i < jumlahRiwayatRelevan; ++i) {
        const RiwayatPembelian& pembelian = larikRiwayatRelevan[i];
        cout << "║ " << setw(2) << nomorUrut++ << " "
             << "║" << left << setw(22) << pembelian.waktuTransaksi
             << "║" << setw(15) << pembelian.namaPelangganTransaksi.substr(0,14)
             << "║" << setw(20) << pembelian.jenisKopiTransaksi.substr(0,19)
             << "║" << setw(16) << FormatRupiah(pembelian.totalHargaTransaksi)
             << "║" << setw(12) << pembelian.jumlahKgTransaksi << " ║\n";
    }
    cout << "╚════╩══════════════════════╩═══════════════╩════════════════════╩════════════════╩═════════════╝\n";

    if (adminBisaHapus && g_penggunaSaatIni.peranAkun == "Admin" && jumlahRiwayatRelevan > 0) {
        int nomorRiwayatUntukDihapus = 0; // Initialize
        if (!AmbilInputInteger(
                "\nMasukkan nomor riwayat yang ingin dihapus (sesuai urutan di atas, 0 untuk batal): ",
                nomorRiwayatUntukDihapus,
                "Input tidak valid! Masukkan angka.",
                "Format nomor tidak valid. Jangan ada karakter tambahan setelah angka."
            )) {
        }


        if (nomorRiwayatUntukDihapus > 0 && nomorRiwayatUntukDihapus <= jumlahRiwayatRelevan) {
            int indeksGlobalUntukDihapus = -1;
            const RiwayatPembelian& itemTargetRef = larikRiwayatRelevan[nomorRiwayatUntukDihapus - 1];

            for(int k=0; k < g_jumlahRiwayatPembelian; ++k) {
                if (g_riwayatPembelian[k].waktuTransaksi == itemTargetRef.waktuTransaksi &&
                    g_riwayatPembelian[k].namaPelangganTransaksi == itemTargetRef.namaPelangganTransaksi &&
                    g_riwayatPembelian[k].jenisKopiTransaksi == itemTargetRef.jenisKopiTransaksi &&
                    abs(g_riwayatPembelian[k].totalHargaTransaksi - itemTargetRef.totalHargaTransaksi) < 0.001 &&
                    g_riwayatPembelian[k].jumlahKgTransaksi == itemTargetRef.jumlahKgTransaksi) {
                    indeksGlobalUntukDihapus = k;
                    break;
                }
            }

            if (indeksGlobalUntukDihapus != -1) {
                cout << "Anda yakin ingin menghapus riwayat no. " << nomorRiwayatUntukDihapus << " ("
                     << g_riwayatPembelian[indeksGlobalUntukDihapus].jenisKopiTransaksi << " oleh "
                     << g_riwayatPembelian[indeksGlobalUntukDihapus].namaPelangganTransaksi << ")? (y/n): ";
                string konfirmasi;
                getline(cin, konfirmasi);

                if (konfirmasi == "y" || konfirmasi == "Y") {
                    for (int i = indeksGlobalUntukDihapus; i < g_jumlahRiwayatPembelian - 1; ++i) {
                        g_riwayatPembelian[i] = g_riwayatPembelian[i+1];
                    }
                    g_jumlahRiwayatPembelian--;
                    SimpanSemuaDataKeJson();
                    cout << "Riwayat pembelian nomor " << nomorRiwayatUntukDihapus << " berhasil dihapus!\n";
                } else {
                    cout << "Penghapusan dibatalkan.\n";
                }
            } else {
                 cout << "Gagal menemukan riwayat yang cocok untuk dihapus (kesalahan internal atau data berubah).\n";
            }
        } else if (nomorRiwayatUntukDihapus != 0) { 
            cout << "Nomor riwayat tidak valid!\n";
        }
    }
    cout.fill(karakterPengisiLama);
    Enter();
}

void TampilkanMenuPenggunaAktif() {
    if (g_penggunaSaatIni.peranAkun == "Admin") MenuAdmin();
    else MenuPelanggan();
}

void ProsesPilihanMenuAdmin(int pilihan) {
    switch(pilihan) {
        case 1: TambahDataKopiBaru(); break;
        case 2: MenuManajemenData(); break;
        case 3: TampilkanRiwayatPenjualan(false); break;
        case 4: LaporanPenjualanl(); break;
        case 5: TampilkanRiwayatPenjualan(true); break;
        case 6: cout << "Log out Admin...\n"; Enter(); g_penggunaSaatIni = Pengguna(); break;
        default:
            TanganiKesalahan("Pilihan menu Admin tidak valid!");
            break;
    }
}

void ProsesPilihanMenuPelanggan(int pilihan) {
    switch(pilihan) {
        case 1: ProsesBeliKopi(); break;
        case 2: CariAsalKopi(); break;
        case 3: LihatSemuaKopi(); Enter(); break;
        case 4: LaporanPenjualanl(); break;
        case 5: TampilkanRiwayatPenjualan(false); break;
        case 6: cout << "Log out Pelanggan...\n"; Enter(); g_penggunaSaatIni = Pengguna(); break;
        default:
            TanganiKesalahan("Pilihan menu Pelanggan tidak valid!");
            break;
    }
}

void DetailPenjualan(const DataPenjualanPerJenis larikPenjualanTerurut[], int jumlahData) {
    cout << "\n╔═══════════════════════════════════════════════╗\n";
    cout << "║         DETAIL PENJUALAN PER JENIS            ║\n";
    cout << "╠════╦═══════════════════════╦══════════════════╣\n";
    cout << "║ No ║     Jenis Kopi        ║ Total Terjual(Kg)║\n";
    cout << "╠════╬═══════════════════════╬══════════════════╣\n";

    if (jumlahData == 0) {
        cout << "║" << left << setw(45) << " Tidak ada data penjualan untuk ditampilkan." << "║\n";
    } else {
        int noUrut = 1;
        for (int i = 0; i < jumlahData; ++i) {
            const DataPenjualanPerJenis& penjualan = larikPenjualanTerurut[i];
            cout << "║ " << left << setw(2) << noUrut++ << " ║ "
                 << left << setw(22) << penjualan.namaKopiTerjual.substr(0,21)
                 << "║ " << right << setw(16) << penjualan.totalKgTerjual << " ║\n";
        }
    }
    cout << "╚════╩═══════════════════════╩══════════════════╝\n";
}


void ProsesRegistrasi(const string& namaPengguna, const string& kataSandi) {
    if (g_jumlahPengguna >= MAKS_PENGGUNA) {
        TanganiKesalahan("Kapasitas pengguna penuh, tidak bisa menambah pengguna baru!");
        return;
    }

    if (namaPengguna.empty() || kataSandi.empty()) {
        TanganiKesalahan("Nama pengguna dan kata sandi tidak boleh kosong!");
        return;
    }
    if (namaPengguna.length() < 3) {
        TanganiKesalahan("Nama pengguna minimal 3 karakter.");
        return;
    }
     if (kataSandi.length() < 3) {
        TanganiKesalahan("Kata sandi minimal 3 karakter.");
        return;
    }


    for (int i = 0; i < g_jumlahPengguna; ++i) {
        if (g_daftarPengguna[i].namaAkun == namaPengguna) {
            TanganiKesalahan("Nama pengguna sudah digunakan!");
            return;
        }
    }

    g_daftarPengguna[g_jumlahPengguna].namaAkun = namaPengguna;
    g_daftarPengguna[g_jumlahPengguna].kataSandiAkun = kataSandi;
    g_daftarPengguna[g_jumlahPengguna].peranAkun = "Pelanggan";
    g_jumlahPengguna++;

    SimpanSemuaDataKeJson();
    cout << "Registrasi berhasil! Silakan login dengan akun baru Anda.\n";
    Enter();
}

bool ProsesLoginPengguna(const string& namaPengguna, const string& kataSandi) {
    if (namaPengguna.empty() || kataSandi.empty()) {
        TanganiKesalahan("Nama pengguna dan kata sandi tidak boleh kosong!");
        return false;
    }

    for (int i = 0; i < g_jumlahPengguna; ++i) {
        if (g_daftarPengguna[i].namaAkun == namaPengguna &&
            g_daftarPengguna[i].kataSandiAkun == kataSandi) {
            g_penggunaSaatIni = g_daftarPengguna[i];
            cout << "Login berhasil sebagai " << (g_penggunaSaatIni.peranAkun == "Admin" ? "Admin" : "Pelanggan") << "!\n";
            Enter();
            return true;
        }
    }

    TanganiKesalahan("Nama pengguna atau kata sandi salah!");
    return false;
}

void MenuLogin() {
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8); 
    #endif
    char karakterPengisiLama = cout.fill();
    cout.fill(' ');
    cout << "\n┌──────────────────────────────────┐\n";
    cout << "│  SISTEM PENJUALAN KOPI INDONESIA │\n";
    cout << "├──────────────────────────────────┤\n";
    cout << "│ 1. Login                         │\n";
    cout << "│ 2. Registrasi Akun               │\n";
    cout << "│ 3. Keluar                        │\n";
    cout << "└──────────────────────────────────┘\n";
    cout.fill(karakterPengisiLama);
}

int main() {
    #ifdef _WIN32 
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    #endif
    MuatSemuaDataDariJson();

    string namaPenggunaInput, kataSandiInput;
    int pilihanMenuUtama;
    bool sudahLogin = false;

    char karakterPengisiLamaDefault = cout.fill(); 

    while (true) {
            cout.fill(' '); 

            #ifdef _WIN32
                system("cls");
            #else
                system("clear");
            #endif

            if (!sudahLogin) {
                MenuLogin();
                if (!AmbilInputInteger("Pilih menu: ", pilihanMenuUtama)) {
                    cout.fill(karakterPengisiLamaDefault); 
                    continue;
                }

                switch(pilihanMenuUtama) {
                    case 1: {
                        system("cls");
                        cout << "\n______________________________________\n";
                        cout << "|                                    |\n";
                        cout << "|           LOGIN PENGGUNA           |\n";
                        cout << "|____________________________________|\n";
                        cout << "Nama Pengguna: "; getline(cin, namaPenggunaInput);
                        namaPenggunaInput = PotongSpasiTepi(namaPenggunaInput);
                        cout << "Kata Sandi: "; getline(cin, kataSandiInput);
                        kataSandiInput = PotongSpasiTepi(kataSandiInput);

                        if (ProsesLoginPengguna(namaPenggunaInput, kataSandiInput)) {
                            sudahLogin = true;
                        }
                        break;
                    }
                    case 2: {
                        system("cls");
                        cout << "\n______________________________________\n";
                        cout << "|                                    |\n";
                        cout << "|       Registrasi Akun Baru         |\n";
                        cout << "|____________________________________|\n";
                        cout << "Nama Pengguna baru: "; getline(cin, namaPenggunaInput);
                        namaPenggunaInput = PotongSpasiTepi(namaPenggunaInput);
                        cout << "Kata Sandi baru: "; getline(cin, kataSandiInput);
                        kataSandiInput = PotongSpasiTepi(kataSandiInput);

                        ProsesRegistrasi(namaPenggunaInput, kataSandiInput);
                        break;
                    }
                    case 3:
                        cout << "Terima kasih telah menggunakan sistem ini!\n";
                        cout.fill(karakterPengisiLamaDefault); 
                        Enter();
                        SimpanSemuaDataKeJson();
                        return 0;
                    default:
                        TanganiKesalahan("Pilihan menu utama tidak valid!");
                        break;
                }
            } else { 
                TampilkanMenuPenggunaAktif();
                int pilihanMenuPengguna;
                if (!AmbilInputInteger("Pilih menu: ", pilihanMenuPengguna)) {
                    cout.fill(karakterPengisiLamaDefault);
                    continue; 
                }

                if (g_penggunaSaatIni.peranAkun == "Admin") {
                    ProsesPilihanMenuAdmin(pilihanMenuPengguna);
                    if (pilihanMenuPengguna == 6 && g_penggunaSaatIni.namaAkun.empty()) sudahLogin = false;
                } else { 
                    ProsesPilihanMenuPelanggan(pilihanMenuPengguna);
                    if (pilihanMenuPengguna == 6 && g_penggunaSaatIni.namaAkun.empty()) sudahLogin = false;
                }
            }
            cout.fill(karakterPengisiLamaDefault); 
    }
    return 0; 
}
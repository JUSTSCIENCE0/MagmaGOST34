#include <iostream>
#include <windows.h>
#include <ctime>

using namespace std;

class MagmaKey 
{
private:
    UINT32* MasterKey = new UINT32[8];

public:
    MagmaKey()
    {
        for (int i = 0; i < 8; i++)
        {
            MasterKey[i] = 0;
            for (int j = 0; j < 4; j++)
            {
                MasterKey[i] <<= 8;
                MasterKey[i] += rand() % 256;
            }
        }
    }

    MagmaKey(UINT64 a, UINT64 b, UINT64 c, UINT64 d)
    {
        for (int i = 0; i < 2; i++)
        {
            MasterKey[1 - i] = (UINT32)a;
            MasterKey[3 - i] = (UINT32)b;
            MasterKey[5 - i] = (UINT32)c;
            MasterKey[7 - i] = (UINT32)d;
            a >>= 32; b >>= 32; c >>= 32; d >>= 32;
        }
    }

    void PrintMasterKey()
    {
        for (int i = 0; i < 8; i++)
        {
            cout << MasterKey[i];
        }
        cout << endl;
    }

    UINT32 GetRoundKey(UINT8 num)
    {
        if (num < 8)
        {
            return MasterKey[num];
        }
        if (num < 16)
        {
            return MasterKey[num - 8];
        }
        if (num < 24)
        {
            return MasterKey[num - 16];
        }
        if (num < 32)
        {
            return MasterKey[31 - num];
        }
        return 0xFFFFFFFF;
    }
};

UINT8 SBox[8][16] = {
    {12, 4, 6, 2, 10, 5, 11, 9, 14, 8, 13, 7, 0, 3, 15, 1},
    {6, 8, 2, 3, 9, 10, 5, 12, 1, 14, 4, 7, 11, 13, 0, 15},
    {11, 3, 5, 8, 2, 15, 10, 13, 14, 1, 7, 4, 12, 9, 6, 0},
    {12, 8, 2, 1, 13, 4, 15, 6, 7, 0, 10, 5, 3, 14, 9, 11},
    {7, 15, 5, 10, 8, 1, 6, 13, 0, 9, 3, 14, 11, 4, 2, 12},
    {5, 13, 15, 6, 9, 2, 12, 10, 11, 7, 8, 1, 4, 3, 14, 0},
    {8, 14, 2, 5, 6, 9, 1, 12, 15, 4, 11, 0, 13, 10, 3, 7},
    {1, 7, 14, 13, 0, 5, 8, 3, 4, 15, 10, 6, 9, 12, 11, 2}
};

UINT64 GenerateUINT64()
{
    UINT64 res = 0;
    for (int i = 0; i < 8; i++)
    {
        res <<= 8;
        res += rand() % 256;
    }
    return res;
}

void BreakingText(UINT64 data, UINT32& l, UINT32& r)
{
    r = (UINT32)data;
    data >>= 32;
    l = (UINT32)data;
}

UINT32 Ttransparent(UINT32 data)
{
    UINT32 res = 0, mask = 0xF0000000, num;
    for (int i = 7; i >= 0; i--)
    {
        res <<= 4;
        num = data & mask;
        num >>= 4 * i;
        res += SBox[i][num];
        mask >>= 4;
    }
    return res;
}

UINT32 CicklSdvig(UINT32 data)
{
    UINT32 res, tmp;
    tmp = data & 0xFFE00000;
    tmp >>= 21;
    res = data << 11;
    res += tmp;
    return res;
}

void GTransparent(UINT32& Left, UINT32& Right, UINT32 Key)
{
    UINT32 tmp = Right;
    Right += Key;
    Right = Ttransparent(Right);
    Right = CicklSdvig(Right);
    Right ^= Left;
    Left = tmp;
}

UINT64 UINT32to64(UINT32 a, UINT32 b)
{
    UINT64 res = a;
    res <<= 32;
    res += b;
    return res;
}

int main()
{
    srand(time(0));

    UINT64 OpenText = GenerateUINT64();
    UINT32 Left, Right;
    cout << hex;
    BreakingText(OpenText, Left, Right);
    cout << "OpenText: " << OpenText << endl;
    cout << "Master Key: ";
    MagmaKey Key = MagmaKey();
    Key.PrintMasterKey();
    cout << endl << "Encryption: " << endl << endl;
    cout << "Start Text:\t" << Left << " " << Right << endl;
    for (int i = 0; i < 31; i++)
    {
        UINT32 RoundKey = Key.GetRoundKey(i);
        GTransparent(Left, Right, RoundKey);
        cout << "After round " << i + 1 << ":\t";
        cout << Left << " " << Right << endl;
    }
    UINT32 tmp = Right;

    UINT32 RoundKey = Key.GetRoundKey(31);
    tmp += RoundKey;
    tmp = Ttransparent(tmp);
    tmp = CicklSdvig(tmp);
    tmp ^= Left;
    Left = tmp;
    cout << "After round " << 32 << ":\t";
    cout << Left << " " << Right << endl;

    UINT64 ClosedText = UINT32to64(Left, Right);
    cout << "Closed Text: " << ClosedText << endl;

    cout << endl << "Decryption: " << endl << endl;
    BreakingText(ClosedText, Left, Right);
    cout << "Start Text:\t" << Left << " " << Right << endl;

    for (int i = 31; i > 0; i--)
    {
        UINT32 RoundKey = Key.GetRoundKey(i);
        GTransparent(Left, Right, RoundKey);
        cout << "After round " << 32 - i << ":\t";
        cout << Left << " " << Right << endl;
    }

    tmp = Right;

    RoundKey = Key.GetRoundKey(0);
    tmp += RoundKey;
    tmp = Ttransparent(tmp);
    tmp = CicklSdvig(tmp);
    tmp ^= Left;
    Left = tmp;
    cout << "After round " << 32 << ":\t";
    cout << Left << " " << Right << endl;

    UINT64 DecryptText = UINT32to64(Left, Right);
    cout << "Decrypted Text: " << DecryptText << endl;

    if (OpenText == DecryptText) cout << "Success!\n";
    else cout << "FAILED!\n";
}

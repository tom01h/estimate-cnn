# BNN と TNN の推論実行環境構築中
学習済みパラメータを使ったバイナリニューラルネットアクセラレータ作成中です
### 準備
- データは[ここ](https://www.cs.toronto.edu/~kriz/cifar.html)から CIFAR-10 binary version (suitable for C programs) の test_batch.bin をダウンロードして cifar10-test にリネーム
- [ここ](https://github.com/tom01h/deep-learning-from-scratch) で作った学習済みパラメータを使う  
  ```$ ../mkparams.pl W1.h mean1.h var1.h W2.h mean2.h var2.h W3.h mean3.h var3.h W4.h mean4.h var4.h W5.h > ../paramn.h
  ```

### C 環境
実行法  
```
$ gcc params2b.c
$ ./a.out > paramb.h
$ gcc estimate.c -lm -o estimate
$ ./estimate
```

- BNN
  - 最初の100データだと 70% の認識精度
  - params.h を変換して paramb.h を作る (params2b.c)
    - mean2,3,4 を x.6 精度の FIXED INT に変換
    - データの並び順を C,Y,X から Y,X,C に変換
  - 2,3,4層のデータ型を整数化済み
  - Norm の直後に BinActiv(符号) なので var は不要
  - Activ 関数を呼んでいるところをすべて BinActiv にする （デフォルト）
- TNN **整数化保留**
  - 最初の100データだと 78% の認識精度
  - ./TNN/params.h.gz を解いて ./params.h にする
  - Activ 関数を呼んでいるところをすべて TriActiv にする
- バッチ処理していないのでとっても遅い

### Verilog 環境
実行法 (Verilatorが必要です)   
```
$ gcc params2mem.c
$ ./a.out > param0
$ make
$ ./sim/Vestimate
```
- BNN
  - params.h を変換して param0 を作る (params2mem.c)
  - 第1層はVerilog化しない
  - 第2,3,4層をVerilog化済み
  - 出力層以降はVerilog化しない
- 真面目にバイナリ化したので C 環境よりも速いみたい
---

##### 推論アクセラレータ
パラメータは内蔵メモリ内に保持して、与えられたアドレスから読みだします。  
入力データは外から与えます。  
入力 x とメモリから読みだすウェイト WEIGHT は 32bit 幅で、入力の1チャンネル分を一度に処理します。  
こんな感じでパイプ化しました。これを32個並列実行します。  
実際には 1st Stage 前半は一つ上の階層にあります。  
![](logic.png)

##### レジスタ
- PARAM (WEIGHT or MEAN)
- ACC
- POOL

##### コマンド
- Init(bias)
  - 2: ACC ← bias (accの要素数)
  - 2: POOL ← -MAX
- Acc(addr, x)
  - 1: WEIGHT ← Mem(addr)
  - 2: ACC ← ACC + Popc(Xnor(WEIGHT, x))
- Pool(bias)
  - 2: POOL ← Max(ACC, POOL)
  - 2: ACC ← bias
- Norm(addr)
  - 1: MEAN ← Mem(addr)
  - 2: POOL ← POOL - MEAN
- Activ()
  - 2: Sign(POOL)

##### 使用法
畳み込み1階層分の計算は
- ストライド繰り返す
  - Init
  - 4回(POOL)繰り返す
    - Acc*9 (畳み込み回数=カーネルサイズ^2)
    - Pool
  - Norm
  - Activ

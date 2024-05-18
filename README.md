<h2 align="center"> Tugas Besar IF2230 Sistem Operasi </h2>

<div align="center">
    <img src="img/title.gif" alt="readme1" width="600"/>
</div>

<br>

<div align="center">
    <img src="img/description.gif" alt="readme2" width="600"/>
</div>

<br>

Hai. Repository ini merupakan hasil Tugas Besar IF2230 Sistem Operasi kami, yaitu membuat sebuah OS (sistem operasi). Sistem operasi ini dibuah khsuusnya untuk platform x86 32-bit protected mode yang dijalankan dengan QEMU.

Projek ini memiliki beberapa milestone, diantaranya:
1. <b> MILESTONE 0 </b> - Toolchain (compiler, assembler, dan linker), Kernel (komponen inti dari sistem operasi), dan implementasi GDT (mendefinisikan segmen memori)
2. <b> MILESTONE 1 </b> - Interrupt (yang memungkinkan CPU merespons dengan cepat terhadap event), Drive (mengelola interaksi perangkat keras, seperti keyboard), File System (implementasi sistem file dasar untuk mengatur penyimpanan dan pengambilan data)
3. <b> MILESTONE 2 </b> - Paging (mengelola alokasi memori), User Mode (mendukung operasi mode pengguna), Shell (menyediakan antarmuka pengguna untuk interaksi sistem dan eksekusi perintah)
4. <b> MILESTONE 3 </b> - Process, Scheduler (mengelola eksekusi proses seperti mengalokasikan waktu CPU di antara beberapa proses), Multitasking (memungkinkan beberapa proses berjalan secara bersamaan)

<br>

<div align="center">
    <img src="img/structure.gif" alt="readme2" width="600"/>
</div>
<br>

Berikut adalah struktur projek kami.
```
│
├── bin
├── img
├── other
├── src
│   ├── build
│   ├── header
│   │   ├── cpu
│   │   ├── driver
│   │   ├── filesystem
│   │   ├── memory
│   │   ├── stdlib
│   │   ├── text
│   │   ├── idt.h
│   │   ├── kernel-entrypoint.h
│   ├── command.c
│   ├── crt0.s
│   ├── disk.c
│   ├── external-inserter.c
│   ├── fat32.c
│   ├── framebuffer.c
│   ├── gdt.c
│   ├── idt.c
│   ├── interrupt.c
│   ├── intsetup.s
│   ├── kernel-entrypoint.s
│   ├── kernel.c
│   ├── keyboard.c
│   ├── linker.ld
│   ├── menu.lst
│   ├── paging.c
│   ├── portio.c
│   ├── stdmem.c
│   ├── string.c
│   ├── user-linker.ld
│   └── user-shell.c
├── .gitignore
├── makefile
└── README.md
```
<br>
<div align="center">
    <img src="img/howtorun.gif" alt="readme2" width="600"/>
</div>
<br>

1. Clone repository ini dengan 
    ```
    git clone https://github.com/labsister21/os-2024-gobyos.git
    ```
2. Buka folder repository pada terminal.
3. Masukkan command `make disk`.
    ```
    make disk
    ```
4. Masukkan command `make insert-shell`.
    ```
    make insert-shell
    ```
4. Terakhir, masukkan command `make` untuk menjalankan sistem operasi.
    ```
    make
    ```

<br>
<div align="center">
    <img src="img/contributors.gif" alt="readme2" width="600"/>
    <br>
    <img src="img/name.gif" alt="readme2" width="600"/>
</div>

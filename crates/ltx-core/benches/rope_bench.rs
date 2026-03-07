use criterion::{Criterion, black_box, criterion_group, criterion_main};
use ltx_core::Buffer;

fn bench_insert_small(c: &mut Criterion) {
    c.bench_function("insert_small_1kb", |b| {
        b.iter(|| {
            let mut buf = Buffer::from_text(&"a".repeat(1024));
            buf.insert(512, black_box("hello world"));
        })
    });
}

fn bench_insert_medium(c: &mut Criterion) {
    let text = "a".repeat(100_000);
    c.bench_function("insert_medium_100kb", |b| {
        b.iter(|| {
            let mut buf = Buffer::from_text(&text);
            buf.insert(50_000, black_box("inserted text here"));
        })
    });
}

fn bench_insert_large(c: &mut Criterion) {
    let text = "a".repeat(10_000_000);
    c.bench_function("insert_large_10mb", |b| {
        b.iter(|| {
            let mut buf = Buffer::from_text(&text);
            buf.insert(5_000_000, black_box("inserted"));
        })
    });
}

fn bench_delete(c: &mut Criterion) {
    let text = "a".repeat(100_000);
    c.bench_function("delete_100kb", |b| {
        b.iter(|| {
            let mut buf = Buffer::from_text(&text);
            buf.delete(10_000, black_box(20_000));
        })
    });
}

fn bench_replace(c: &mut Criterion) {
    let text = "a".repeat(100_000);
    c.bench_function("replace_100kb", |b| {
        b.iter(|| {
            let mut buf = Buffer::from_text(&text);
            buf.replace(10_000, 20_000, black_box("replacement"));
        })
    });
}

fn bench_undo_redo(c: &mut Criterion) {
    c.bench_function("undo_redo_cycle", |b| {
        b.iter(|| {
            let mut buf = Buffer::new();
            for i in 0..100 {
                buf.insert(i, "x");
            }
            for _ in 0..100 {
                buf.undo();
            }
            for _ in 0..100 {
                buf.redo();
            }
        })
    });
}

criterion_group!(
    benches,
    bench_insert_small,
    bench_insert_medium,
    bench_insert_large,
    bench_delete,
    bench_replace,
    bench_undo_redo,
);
criterion_main!(benches);

# Passing a struct by value

Clang 6.

```c
struct Point {
    int x;
    int y;
    int z;
    int a;
    int b;
    int c;

    Point() = default;
    Point(int x, int y): x(x), y(y){};
};

Point copy_point(Point p)
{
    auto lp = Point();
    lp.x = p.x;
    lp.y = p.y;
    return lp;
}

int square(int num) {
    auto b = Point();
    b.x = 12;
    b.y = 8;

    auto c = copy_point(b);

    return 1;
}
```

```
%struct.Point = type { i32, i32, i32, i32, i32, i32 }

define void @_Z10copy_point5Point(%struct.Point* noalias sret, %struct.Point* byval align 8) #0 !dbg !7 {
  call void @llvm.dbg.declare(metadata %struct.Point* %1, metadata !26, metadata !DIExpression()), !dbg !27
  call void @llvm.dbg.declare(metadata %struct.Point* %0, metadata !28, metadata !DIExpression()), !dbg !29
  %3 = bitcast %struct.Point* %0 to i8*, !dbg !29
  call void @llvm.memset.p0i8.i64(i8* %3, i8 0, i64 24, i32 4, i1 false), !dbg !29
  %4 = getelementptr inbounds %struct.Point, %struct.Point* %1, i32 0, i32 0, !dbg !30
  %5 = load i32, i32* %4, align 8, !dbg !30
  %6 = getelementptr inbounds %struct.Point, %struct.Point* %0, i32 0, i32 0, !dbg !31
  store i32 %5, i32* %6, align 4, !dbg !32
  %7 = getelementptr inbounds %struct.Point, %struct.Point* %1, i32 0, i32 1, !dbg !33
  %8 = load i32, i32* %7, align 4, !dbg !33
  %9 = getelementptr inbounds %struct.Point, %struct.Point* %0, i32 0, i32 1, !dbg !34
  store i32 %8, i32* %9, align 4, !dbg !35
  ret void, !dbg !36
}

declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i32, i1) #2

define i32 @_Z6squarei(i32) #0 !dbg !37 {
  %2 = alloca i32, align 4
  %3 = alloca %struct.Point, align 4
  %4 = alloca %struct.Point, align 4
  %5 = alloca %struct.Point, align 8
  store i32 %0, i32* %2, align 4
  call void @llvm.dbg.declare(metadata i32* %2, metadata !40, metadata !DIExpression()), !dbg !41
  call void @llvm.dbg.declare(metadata %struct.Point* %3, metadata !42, metadata !DIExpression()), !dbg !43
  %6 = bitcast %struct.Point* %3 to i8*, !dbg !43
  call void @llvm.memset.p0i8.i64(i8* %6, i8 0, i64 24, i32 4, i1 false), !dbg !43
  %7 = getelementptr inbounds %struct.Point, %struct.Point* %3, i32 0, i32 0, !dbg !44
  store i32 12, i32* %7, align 4, !dbg !45
  %8 = getelementptr inbounds %struct.Point, %struct.Point* %3, i32 0, i32 1, !dbg !46
  store i32 8, i32* %8, align 4, !dbg !47
  call void @llvm.dbg.declare(metadata %struct.Point* %4, metadata !48, metadata !DIExpression()), !dbg !49
  %9 = bitcast %struct.Point* %5 to i8*, !dbg !50
  %10 = bitcast %struct.Point* %3 to i8*, !dbg !50
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %9, i8* %10, i64 24, i32 4, i1 false), !dbg !50
  call void @_Z10copy_point5Point(%struct.Point* sret %4, %struct.Point* byval align 8 %5), !dbg !51
  ret i32 1, !dbg !52
}

declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i32, i1) #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { argmemonly nounwind }
```
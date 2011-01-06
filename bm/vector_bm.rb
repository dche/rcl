# encoding: utf-8

has_narray = true

begin
  require 'narray'
rescue LoadError => e
  puts "You need to install the 'narray' gem to compare the benchmarks with NArray."
  has_narray = false
end

require File.join(File.dirname(__FILE__), 'bm_helper')
require File.join(File.dirname(__FILE__), '../lib/opencl/vector')

def test(title)
  puts "\n== #{title} #{'=' * 20}"
end

(16..19).each do |i|

  sz = 2 ** i

  puts "-" * 20 + " #{sz} " + "-" * 20
  ra = Array.new(sz, 0)
  na = NArray.float(sz) if has_narray
  vec = OpenCL::Vector.ones(sz)

  test "Random filling"
  bm(8) do |b|
    b.report('Array') do
      sz.times do |i|; ra[i] = Kernel.rand; end
    end

    b.report('NArray') do
      na.random!
    end if has_narray

    b.report('OpenCL') do
      vec.rand
    end
  end

  test "Multiply a number."

  n = Kernel.rand

  bm(8) do |b|
    b.report('Array') do
      ra.map do |i|; ra[i] *= n; end
    end

    b.report('NArray') do
      na * n
    end if has_narray

    b.report('OpenCL') do
      vec * n
    end
  end

  test "Summary"
  bm(8) do |b|
    b.report('Array') do
      ra.reduce(:+)
    end

    b.report('NArray') do
      na.sum
    end if has_narray

    b.report('OpenCL') do
      vec.summary
    end
  end

  test 'Min'
  bm(8) do |b|
    b.report('Array') do
      ra.min
    end

    b.report('NArray') do
      na.min
    end if has_narray

    b.report('OpenCL') do
      vec.min
    end
  end

  test 'Max'
  bm(8) do |b|
    b.report('Array') do
      ra.max
    end

    b.report('NArray') do
      na.max
    end if has_narray

    b.report('OpenCL') do
      vec.max
    end
  end

  test "mad with number"
  m = Kernel.rand
  bm(8) do |b|
    b.report('Array') do
      ra.map do |i|; ra[i] * m + n; end
    end

    b.report('NArray') do
      na * m + n
    end if has_narray

    b.report('OpenCL') do
      vec.mad(m, n)
    end
  end

  test "mad with vector"

  mul_ra = Array.new(sz, m)
  add_ra = Array.new(sz, n)

  if has_narray
    mul_na = NArray.float(sz) + m
    add_na = NArray.float(sz) + n
  end

  mul_vec = OpenCL::Vector.new(sz).fill(m)
  add_vec = OpenCL::Vector.new(sz).fill(n)
  bm(8) do |b|
    b.report('Array') do
      ra.map do |i|; ra[i] * mul_ra[i] + add_ra[i]; end
    end

    b.report('NArray') do
      (na * mul_na + add_na)
    end if has_narray

    b.report('OpenCL') do
      vec.mad(mul_vec, add_vec)
    end
  end

  test 'Multiple operations in sequence.'
  bm(8) do |b|
    b.report('NArray') do
      60.times do
        ((na * mul_na + add_na).random * n + mul_na) * mul_na + m
      end
    end if has_narray

    b.report('OpenCL') do
      60.times do
        (vec.mad(mul_vec, add_vec) * n + mul_vec) * mul_vec + m
      end
    end
  end

  test 'Trigonometric functions.'
  bm(8) do |b|
    b.report('Ruby') do
      10.times do
        ra.map do |n|
          Math.sin n
        end.map do |n|
          Math.tan n
        end.map do |n|
          Math.cos n
        end.map do |n|
          Math.tan n
        end
      end
    end

    b.report('NArray') do
      10.times do
        NMath.tan(NMath.cos(NMath.tan(NMath.sin(na))))
      end
    end if has_narray

    b.report('OpenCL') do
      10.times do
        vec.sin.tan.cos.tan
      end
    end
  end

  test 'Mean'
  bm(8) do |b|

    b.report('NArray') do
      na.mean
    end if has_narray

    b.report('OpenCL') do
      vec.mean
    end
  end

end

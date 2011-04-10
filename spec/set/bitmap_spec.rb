# encoding: utf-8

require File.join(File.dirname(__FILE__), '../spec_helper')
require File.join(File.dirname(__FILE__), '../../lib/opencl/set')

describe Bitmap do
  the '::new' do
    bm = Bitmap.new 2
    bm.length.should.equal 1
    bm.type.tag.should.equal :cl_uint
    bm.byte_size.should.equal OpenCL.type_size(:cl_uint)
    bm.count.should.equal 0

    bm = Bitmap.new 33
    bm.length.should.equal 2
  end

  the 'Bitmap object should not respond to NDArray\'s methods' do
    bm = Bitmap.new 2
    bm.should.respond_to :sum
    bm.should.respond_to :bits
    bm.should.not.respond_to :fill
  end

  the '#next_cell should not exceeds capacity.' do
    bm = Bitmap.new 4
    64.times do
      bm.next_cell.should < 4
    end
  end

  the '#mark_cell' do
    bm = Bitmap.new 4

    i = bm.next_cell
    bm.set?(i).should.not.be.true
    bm.mark_cell i
    bm.set?(i).should.be.true
  end

  the '#next_cell should not allocate marked cell.' do
    bm = Bitmap.new 4

    i = bm.next_cell
    bm.mark_cell i
    64.times do
      bm.next_cell.should.not.equal i
    end
  end

  it 'should.raise RuntimeError if capacity if full.' do
    bm = Bitmap.new 4

    should.not.raise(Exception) do
      4.times do
        bm.mark_cell bm.next_cell
      end
    end
    bm.count.should.equal 4

    should.raise(RuntimeError) do
      bm.next_cell
    end
  end

  the 'duplicated #mark_cell is harmless.' do
    bm = Bitmap.new 4

    i = bm.next_cell
    4.times do
      i = bm.next_cell
      bm.mark_cell i
    end

    should.not.raise(Exception) do
      bm.mark_cell i
    end
  end

end

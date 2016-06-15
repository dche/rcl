
require File.join(File.dirname(__FILE__), '../spec_helper')
require File.join(File.dirname(__FILE__), '../../lib/opencl/set')

describe Bitmap do
  the '::new' do
    bm = Bitmap.new 2
    bm.length.should.equal 2
    bm.type.tag.should.equal :cl_uint
    bm.count.should.equal 0

    bm = Bitmap.new 33
    bm.length.should.equal 3
  end

  the 'Bitmap object should not respond to NDArray\'s methods' do
    bm = Bitmap.new 2
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

  the '#clear_cell' do
    bm = Bitmap.new 4
    i = bm.next_cell
    bm.mark_cell i
    bm.should.be.set i
    bm.count.should.equal 1
    bm.clear_cell i
    bm.count.should.equal 0
    bm.should.not.be.set i
  end

  the 'clear free bit should be harmless.' do
    bm = Bitmap.new 32, :linear
    i = bm.next_cell
    bm.mark_cell i
    bm.count.should.equal 1
    10.times { bm.clear_cell i + 1 }
    bm.count.should.equal 1
    bm.clear_cell i
    bm.count.should.equal 0
    bm.should.not.be.set i
    bm.mark_cell bm.next_cell
    bm.count.should.equal 1
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

  the '#recount' do
    bm = Bitmap.new 1024
    981.times do
      bm.mark_cell bm.next_cell
    end
    bm.count.should.equal 981
    bm.recount.should.equal 981
    bm.count.should.equal 981
  end

  the 'linear allocation' do
    bm = Bitmap.new 2048, :linear
    2016.times do
      bm.mark_cell bm.next_cell
    end
    bm.count.should.equal 2016
    bm.recount.should.equal 2016
    10.times { bm.next_cell.should.equal 2016 }
    bm.send :[]=, 63, 0xf0f0f0f3
    10.times { bm.next_cell.should.equal 2018 }
    bm.send :[]=, 0, 0xf0f0f0f2
    bm.send :[]=, 63, 0xFFFFFFFF
    10.times { bm.next_cell.should.equal 0 }
    bm.mark_cell bm.next_cell
    bm.next_cell.should.equal 2
  end
end

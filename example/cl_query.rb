#! /usr/bin/env ruby
# encoding: utf-8

require File.join(File.dirname(__FILE__), '../lib/opencl')

def pinfo(str, value)
  print str.rjust 28
  print ': '
  puts value
end

puts '=' * 60

OpenCL::Capi.platforms.each do |pt|
  pinfo "Platform", "#{pt.name}"
  pinfo "Vendor", "#{pt.vendor}"
  pinfo "Version", "#{pt.version}"

  OpenCL::Capi.devices(OpenCL::Capi::CL_DEVICE_TYPE_ALL, pt).each do |dev|
    puts '-' * 60

    pinfo "Device", "#{dev.name}"
    pinfo "Version", "#{dev.version}"
    pinfo "Type", "#{dev.gpu? ? 'GPU' : 'CPU'}"
    pinfo "Endian", "#{dev.big_endian? ? 'Big' : 'Little'}"
    pinfo "Max Clock Freqency", "#{dev.max_clock_frequency}M Hz"
    pinfo "Compute Units", "#{dev.max_compute_units}"
    pinfo "Address Width", "#{dev.address_bits} bits"
    pinfo "Base Address Align", "#{dev.base_address_align}"
    pinfo "Global Memory Size", "#{dev.global_memory_size / (1024 ** 2)}M bytes"
    pinfo "Local Memory Size", "#{dev.local_memory_size / 1024}K bytes"
    pinfo "Max Memory Allocation Size", "#{dev.max_mem_alloc_size / (1024 ** 2)}M bytes"
    pinfo "Max Work Item Sizes", "#{dev.max_work_item_sizes}"
    pinfo "Max Workgroup Size", "#{dev.max_workgroup_size}"
    pinfo "Available", "#{dev.available? ? 'Yes' : 'No'}"
  end
  puts '=' * 60
end

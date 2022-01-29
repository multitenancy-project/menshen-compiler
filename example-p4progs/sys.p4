#include <core.p4>
#include <fpga.p4>

header ethernet_t {
    bit<48> eth_dst_addr;
    bit<48> eth_src_addr;
    bit<16> eth_ethertype;
}

header vlan_t {
	bit<16> vlan_id;
	bit<16> vlan_ethertype;
}

header ipv4_t {
    bit<4>  version;
    bit<4>  ihl;
    bit<8>  diffserv;
    bit<16> total_len;
    bit<16> identification;
    bit<3>  flags;
    bit<13> frag_offset;
    bit<8>  ttl;
    bit<8>  protocol;
    bit<16> ip_checksum;
    bit<32> ip_src_addr;
    bit<32> ip_dst_addr;
}

header udp_t {
	bit<16> udp_src_port;
    bit<16> udp_dst_port;
    bit<16> hdr_length;
    bit<16> udp_checksum;
}

struct headers {
    ethernet_t   ethernet;
	vlan_t       vlan;
	ipv4_t		 ipv4;
	udp_t        udp;
}

struct metadata {
    /* In our case it is empty */
}

/*************************************************************************
 ***********************  P A R S E R  ***********************************
 *************************************************************************/
parser MyParser(packet_in packet,
                out headers hdr,
                inout metadata meta,
                inout standard_metadata_t standard_metadata) {

    state start {
        packet.extract(hdr.ethernet);
		transition parse_vlan;
    }

	state parse_vlan {
		packet.extract(hdr.vlan);
		transition parse_ip;
	}

	state parse_ip {
		packet.extract(hdr.ipv4);
		transition parse_udp;
	}

	state parse_udp {
		packet.extract(hdr.udp);
		transition accept;
	}
    
}

/*************************************************************************
 **************  I N G R E S S   P R O C E S S I N G   *******************
 *************************************************************************/
control MyIngress(inout headers hdr,
                  inout metadata meta,
                  inout standard_metadata_t standard_metadata) {

	action fwd_on_ip_dst () {
		standard_metadata.port = 4;
	}

	action fwd_on_ip_multicast() {
		standard_metadata.port = 85;
	}

    table sys_fwd_on_ip_dst {
        key = {
            hdr.ipv4.ip_dst_addr: exact;
        }
        actions = {
			fwd_on_ip_dst;
			fwd_on_ip_multicast;
        }
        const default_action = fwd_on_ip_dst();
        const entries = {
            9: fwd_on_ip_dst();
			10: fwd_on_ip_multicast();
        }
    }

    apply {
        sys_fwd_on_ip_dst.apply();
    }
}

/*************************************************************************
 ***********************  S W I T T C H **********************************
 *************************************************************************/

FpgaSwitch(
MyParser(),
MyIngress()
) main;

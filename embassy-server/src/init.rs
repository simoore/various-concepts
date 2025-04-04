use core::net::Ipv4Addr;

use embassy_executor::Spawner;
use embassy_net::{Ipv4Cidr, StackResources};
use embassy_stm32::eth::generic_smi::GenericSMI;
use embassy_stm32::eth::{
    CRSPin, Ethernet, MDCPin, MDIOPin, PacketQueue, RXD0Pin, RXD1Pin, RefClkPin, TXD0Pin, TXD1Pin,
    TXEnPin,
};
use embassy_stm32::peripherals::ETH;
use embassy_stm32::rcc::{
    AHBPrescaler, APBPrescaler, HSIPrescaler, Pll, PllDiv, PllMul, PllPreDiv, PllSource, Sysclk,
    VoltageScale,
};
use embassy_stm32::{Config, Peripheral, Peripherals};
use heapless::Vec;

use crate::static_cell::StaticCell;


#[unsafe(link_section = ".dma_mem_d2")]
static PACKETS: StaticCell<PacketQueue<4, 4>> = StaticCell::new();

static RESOURCES: StaticCell<StackResources<3>> = StaticCell::new();

type Device = Ethernet<'static, ETH, GenericSMI>;

embassy_stm32::bind_interrupts!(struct Irqs {
    ETH => embassy_stm32::eth::InterruptHandler;
});

/// This task services the network stack.
/// 
/// @param runner
///     The handle to service the network stack.
#[embassy_executor::task]
async fn net_task(mut runner: embassy_net::Runner<'static, Device>) -> ! {
    runner.run().await
}

/// Initializes the clock system of the MCU.
pub fn init_mcu() -> Peripherals {
    let mut config = Config::default();
    config.rcc.hsi = Some(HSIPrescaler::DIV1);
    config.rcc.csi = true;
    config.rcc.hsi48 = Some(Default::default()); // needed for RNG
    config.rcc.pll1 = Some(Pll {
        source: PllSource::HSI,
        prediv: PllPreDiv::DIV4,
        mul: PllMul::MUL50,
        divp: Some(PllDiv::DIV2),
        divq: None,
        divr: None,
    });
    config.rcc.sys = Sysclk::PLL1_P; // 400 Mhz
    config.rcc.ahb_pre = AHBPrescaler::DIV2; // 200 Mhz
    config.rcc.apb1_pre = APBPrescaler::DIV2; // 100 Mhz
    config.rcc.apb2_pre = APBPrescaler::DIV2; // 100 Mhz
    config.rcc.apb3_pre = APBPrescaler::DIV2; // 100 Mhz
    config.rcc.apb4_pre = APBPrescaler::DIV2; // 100 Mhz
    config.rcc.voltage_scale = VoltageScale::Scale1;
    embassy_stm32::init(config)
}

/// Initializes the network stack.
#[allow(clippy::too_many_arguments)]
pub async fn init_network(
    spawner: &Spawner,
    eth: ETH,
    ref_clk: impl Peripheral<P = impl RefClkPin<ETH>> + 'static,
    mdio: impl Peripheral<P = impl MDIOPin<ETH>> + 'static,
    mdc: impl Peripheral<P = impl MDCPin<ETH>> + 'static,
    crs: impl Peripheral<P = impl CRSPin<ETH>> + 'static,
    rx_d0: impl Peripheral<P = impl RXD0Pin<ETH>> + 'static,
    rx_d1: impl Peripheral<P = impl RXD1Pin<ETH>> + 'static,
    tx_d0: impl Peripheral<P = impl TXD0Pin<ETH>> + 'static,
    tx_d1: impl Peripheral<P = impl TXD1Pin<ETH>> + 'static,
    tx_en: impl Peripheral<P = impl TXEnPin<ETH>> + 'static,
) -> embassy_net::Stack<'static> {

    defmt::info!("packets address: {=usize}", &PACKETS as *const _ as usize);
    defmt::info!("resources address: {=usize}", &RESOURCES as *const _ as usize);

    let mac_addr = [0x00, 0x00, 0xDE, 0xAD, 0xBE, 0xEF];
    let device = Ethernet::new(
        PACKETS.init(PacketQueue::new()),
        eth,
        Irqs,
        ref_clk,
        mdio,
        mdc,
        crs,
        rx_d0,
        rx_d1,
        tx_d0,
        tx_d1,
        tx_en,
        GenericSMI::new(0),
        mac_addr,
    );

    let config = embassy_net::Config::ipv4_static(embassy_net::StaticConfigV4 {
        address: Ipv4Cidr::new(Ipv4Addr::new(192, 168, 10, 3), 24),
        dns_servers: Vec::new(),
        gateway: Some(Ipv4Addr::new(192, 168, 10, 1)),
    });

    // Init network stack
    let (stack, runner) =
        embassy_net::new(device, config, RESOURCES.init(StackResources::new()), 1);

    // Launch network task
    spawner
        .spawn(net_task(runner))
        .expect("Unable to launch net task");

    // Ensure DHCP configuration is up before trying connect
    stack.wait_config_up().await;

    defmt::info!("Network task initialized");
    stack
}

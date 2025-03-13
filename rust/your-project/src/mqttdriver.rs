use embassy_net::{
    tcp::{ConnectError, TcpSocket},
    IpAddress, Stack,
};
use log::info;
use rust_mqtt::{
    client::{
        client::MqttClient,
        client_config::{ClientConfig, MqttVersion},
    },
    packet::v5::{publish_packet::QualityOfService, reason_codes::ReasonCode},
    utils::rng_generator::CountingRng,
};
use serde::{Deserialize, Serialize};
use static_cell::StaticCell;

static mut RECV_BUFFER: [u8; 4096] = [0; 4096];
static mut WRITE_BUFFER: [u8; 4096] = [0; 4096];
static mut RX_BUFFER: [u8; 4096] = [0; 4096];
static mut TX_BUFFER: [u8; 4096] = [0; 4096];
static mut SOCKET: StaticCell<TcpSocket> = StaticCell::new();

#[derive(Serialize, Deserialize)]
pub struct Message {
    distance: f64,
    cpu_usage: f64,
}

impl Message {
    pub fn new(distance: f64, cpu_usage: f64) -> Self {
        Self {
            distance,
            cpu_usage,
        }
    }
}

#[derive(Default)]
pub struct MqttDriverBuilder<'a> {
    id: Option<&'a str>,
    username: Option<&'a str>,
    pwd: Option<&'a str>,
    keep_alive: Option<u16>,
    packet_size: Option<u32>,
    broker: Option<(IpAddress, u16)>,
}

impl<'a> MqttDriverBuilder<'a> {
    pub fn with_id(mut self, id: &'a str) -> Self {
        self.id = Some(id);
        self
    }

    pub fn with_username(mut self, username: &'a str) -> Self {
        self.username = Some(username);
        self
    }

    pub fn with_password(mut self, pwd: &'a str) -> Self {
        self.pwd = Some(pwd);
        self
    }

    pub fn with_keep_alive(mut self, t: u16) -> Self {
        self.keep_alive = Some(t);
        self
    }

    pub fn with_packet_size(mut self, s: u32) -> Self {
        self.packet_size = Some(s);
        self
    }

    pub fn with_broker_ip(mut self, ip: IpAddress, port: u16) -> Self {
        self.broker = Some((ip, port));
        self
    }

    pub async fn build(self, stack: Stack<'static>) -> Result<MqttDriver<'a>, ConnectError> {
        let mut conf: ClientConfig<'_, 5, CountingRng> =
            ClientConfig::new(MqttVersion::MQTTv5, CountingRng(20000));
        conf.add_client_id(self.id.unwrap());
        conf.add_username(self.username.unwrap());
        conf.add_password(self.pwd.unwrap());
        conf.max_packet_size = self.packet_size.unwrap();
        conf.keep_alive = self.keep_alive.unwrap();

        let socket = unsafe { SOCKET.init(TcpSocket::new(stack, &mut RX_BUFFER, &mut TX_BUFFER)) };

        let status = socket.connect(self.broker.unwrap()).await;

        match status {
            Ok(_) => Ok(MqttDriver {
                client: MqttClient::new(
                    socket,
                    unsafe { &mut WRITE_BUFFER },
                    4096,
                    unsafe { &mut RECV_BUFFER },
                    4096,
                    conf,
                ),
            }),
            Err(err) => Err(err),
        }
    }
}

pub struct MqttDriver<'a> {
    client: MqttClient<'a, &'a mut TcpSocket<'static>, 5, CountingRng>,
}

impl<'a> MqttDriver<'a> {
    pub fn new() -> MqttDriverBuilder<'a> {
        MqttDriverBuilder::default()
    }

    pub async fn connect(&mut self) -> Result<(), ReasonCode> {
        self.client.connect_to_broker().await
    }

    pub async fn publish(&mut self, message: Message) -> Result<(), ReasonCode> {
        let mut json = [0u8; 248];
        let _len = serde_json_core::to_slice(&message, &mut json).unwrap();
        let a = core::str::from_utf8(&json).unwrap();
        self.client
            .send_message("data", a.as_bytes(), QualityOfService::QoS0, false)
            .await
    }
}

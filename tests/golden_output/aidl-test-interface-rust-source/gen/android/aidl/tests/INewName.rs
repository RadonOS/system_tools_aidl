#![allow(non_upper_case_globals)]
#![allow(non_snake_case)]
#[allow(unused_imports)] use binder::IBinder;
use binder::declare_binder_interface;
declare_binder_interface! {
  INewName["android.aidl.tests.IOldName"] {
    native: BnNewName(on_transact),
    proxy: BpNewName {
    },
  }
}
pub trait INewName: binder::Interface + Send {
  fn get_descriptor() -> &'static str where Self: Sized { "android.aidl.tests.IOldName" }
  fn RealName(&self) -> binder::public_api::Result<String> {
    Err(binder::StatusCode::UNKNOWN_TRANSACTION.into())
  }
  fn getDefaultImpl() -> INewNameDefault where Self: Sized {
    DEFAULT_IMPL.lock().unwrap().clone()
  }
  fn setDefaultImpl(d: INewNameDefault) -> INewNameDefault where Self: Sized {
    std::mem::replace(&mut *DEFAULT_IMPL.lock().unwrap(), d)
  }
}
pub mod transactions {
  #[allow(unused_imports)] use binder::IBinder;
  pub const RealName: binder::TransactionCode = binder::SpIBinder::FIRST_CALL_TRANSACTION + 0;
}
pub type INewNameDefault = Option<std::sync::Arc<dyn INewName + Sync>>;
use lazy_static::lazy_static;
lazy_static! {
  static ref DEFAULT_IMPL: std::sync::Mutex<INewNameDefault> = std::sync::Mutex::new(None);
}
pub(crate) mod mangled { pub use super::INewName as _7_android_4_aidl_5_tests_8_INewName; }
impl INewName for BpNewName {
  fn RealName(&self) -> binder::public_api::Result<String> {
    let _aidl_reply = self.binder.transact(transactions::RealName, 0, |_aidl_data| {
      Ok(())
    });
    if let Err(binder::StatusCode::UNKNOWN_TRANSACTION) = _aidl_reply {
      if let Some(_aidl_default_impl) = <Self as INewName>::getDefaultImpl() {
        return _aidl_default_impl.RealName();
      }
    }
    let _aidl_reply = _aidl_reply?;
    let _aidl_status: binder::Status = _aidl_reply.read()?;
    if !_aidl_status.is_ok() { return Err(_aidl_status); }
    let _aidl_return: String = _aidl_reply.read()?;
    Ok(_aidl_return)
  }
}
impl INewName for binder::Binder<BnNewName> {
  fn RealName(&self) -> binder::public_api::Result<String> { self.0.RealName() }
}
fn on_transact(_aidl_service: &dyn INewName, _aidl_code: binder::TransactionCode, _aidl_data: &binder::parcel::Parcel, _aidl_reply: &mut binder::parcel::Parcel) -> binder::Result<()> {
  match _aidl_code {
    transactions::RealName => {
      let _aidl_return = _aidl_service.RealName();
      match &_aidl_return {
        Ok(_aidl_return) => {
          _aidl_reply.write(&binder::Status::from(binder::StatusCode::OK))?;
          _aidl_reply.write(_aidl_return)?;
        }
        Err(_aidl_status) => _aidl_reply.write(_aidl_status)?
      }
      Ok(())
    }
    _ => Err(binder::StatusCode::UNKNOWN_TRANSACTION)
  }
}
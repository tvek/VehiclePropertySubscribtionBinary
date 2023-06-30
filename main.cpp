#include <iostream>
#include <android/hardware/automotive/vehicle/2.0/IVehicle.h>
#include <android-base/macros.h>  
/*
#include <sys/system_properties.h>
int main(){
    std::cout << "Hello World from God" << std::endl;
    char sdk_ver_str[100];
    __system_property_get("ro.build.version.sdk", sdk_ver_str);
    std::cout << "Property = " <<  sdk_ver_str << std::endl;
}
*/
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_handle;
using ::android::sp;
using ::android::wp;
using namespace ::android::hardware::automotive::vehicle::V2_0;

class ThomasVehicleListener : public IVehicleCallback {
public:
    // Methods from ::android::hardware::automotive::vehicle::V2_0::IVehicleCallback follow.
    Return<void> onPropertyEvent(const hidl_vec <VehiclePropValue> & values) override {
        {
            // Our use case is so simple, we don't actually need to update a variable,
            // but the docs seem to say we have to take the lock anyway to keep
            // the condition variable implementation happy.
            std::lock_guard<std::mutex> g(mLock);
            std::cout << "onPropertyEvent changed properties count " << values.size() << std::endl;
            if (0 < values.size()) {
                if (0 < values[0].value.int32Values.size()) {
                    std::cout << "onPropertyEvent int32Values " << values[0].value.int32Values[0] << std::endl;
                } else {
                    std::cout << "onPropertyEvent int32Values size is zero" << std::endl;
                }
                
            } else {
                std::cout << "onPropertyEvent changed properties count is zero" << std::endl;
            }
        }
        mEventCond.notify_one();
        return Return<void>();
    }

    Return<void> onPropertySet(const VehiclePropValue & /*value*/) override {
        // Ignore the direct set calls (we don't expect to make any anyway)
        return Return<void>();
    }

    Return<void> onPropertySetError(StatusCode      /* errorCode */,
                                    int32_t         /* propId */,
                                    int32_t         /* areaId */) override {
        // We don't set values, so we don't listen for set errors
        return Return<void>();
    }

    bool waitForEvents(int timeout_ms) {
        std::unique_lock<std::mutex> g(mLock);
        std::cv_status result = mEventCond.wait_for(g, std::chrono::milliseconds(timeout_ms));
        return (result == std::cv_status::no_timeout);
    }

    void run() {
        while (true) {
            // Wait until we have an event to which to react
            // (wake up and validate our current state "just in case" every so often)
            waitForEvents(5000);

            // If we were delivered an event (or it's been a while) update as necessary
            
        }
    }

private:
    std::mutex mLock;
    std::condition_variable mEventCond;
};

int main(){
    std::cout << "Hello World" << std::endl;

    sp<ThomasVehicleListener> listener = new ThomasVehicleListener(); 
    sp<IVehicle> pVnet = IVehicle::getService();
    if (nullptr == pVnet.get()) {
        std::cout << "pVnet is null" << std::endl;
    } else {
        SubscribeOptions optionsData[] = {
            {
                .propId = static_cast<int32_t>(VehicleProperty::GEAR_SELECTION),
                .flags  = SubscribeFlags::EVENTS_FROM_CAR
            },
        };
        hidl_vec <SubscribeOptions> options;
        options.setToExternal(optionsData, arraysize(optionsData));
        StatusCode status = pVnet->subscribe(listener, options);
        if (status != StatusCode::OK) {
            std::cout << "VHAL subscription for property failed with code " << static_cast<int32_t>(status);
        }

        std::cout << "HAL waiting started " << std::endl;
        listener->run();
    }
    std::cout << "Program Terminated" << std::endl;
    
}
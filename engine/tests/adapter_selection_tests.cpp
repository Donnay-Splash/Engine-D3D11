#include "Engine.h"
#include "EngineFactory.h"
#include "Graphics/Adapter.h"
#include "Graphics/Device.h"
#include "Graphics/GraphicsInstance.h"
#include "ShaderLibrary.h"

#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {

class TestAdapter final : public Engine::Graphics::Adapter
{
public:
    explicit TestAdapter(std::vector<Engine::Graphics::QueueType> queues)
    {
        m_Capabilities.queueTypes = std::move(queues);
        m_Capabilities.name = "test adapter";
    }

    const Engine::Graphics::DeviceCapabilities &Capabilities() const noexcept override
    {
        return m_Capabilities;
    }

    std::shared_ptr<Engine::Graphics::Device> CreateDevice(Engine::Error &error) override
    {
        ++createDeviceCalls;
        error.message = "test device creation";
        return nullptr;
    }

    std::uint32_t createDeviceCalls = 0;

private:
    Engine::Graphics::DeviceCapabilities m_Capabilities;
};

class TestGraphicsInstance final : public Engine::Graphics::GraphicsInstance
{
public:
    explicit TestGraphicsInstance(std::vector<std::shared_ptr<Engine::Graphics::Adapter>> adapters)
        : m_Adapters(std::move(adapters))
    {
    }

    bool EnumerateAdapters(
        std::vector<std::shared_ptr<Engine::Graphics::Adapter>> &adapters,
        Engine::Error &) override
    {
        adapters = m_Adapters;
        return true;
    }

private:
    std::vector<std::shared_ptr<Engine::Graphics::Adapter>> m_Adapters;
};

class TestShaderLibrary final : public Engine::ShaderLibrary
{
public:
    bool RequestGraphicsShaders(
        Engine::Graphics::Device &,
        Engine::ShaderProgramType,
        const std::string &,
        Engine::GraphicsShaderPair &,
        Engine::Error &) override
    {
        return false;
    }
};

class TestSurface final : public Engine::Surface
{
public:
    Engine::Size DrawableSize() const noexcept override
    {
        return {1, 1};
    }

    bool Start(const Callbacks &, Engine::Error &) override
    {
        return true;
    }

    void Stop() noexcept override
    {
    }
};

bool InitializeWith(
    const std::shared_ptr<Engine::Graphics::GraphicsInstance> &instance,
    const std::shared_ptr<Engine::Graphics::Adapter> &preferredAdapter,
    Engine::Error &error)
{
    std::unique_ptr<Engine::Engine> engine = Engine::EngineFactory::create(
        instance, std::make_shared<TestShaderLibrary>(), preferredAdapter);
    TestSurface surface;
    return engine->Initialize(surface, {}, error);
}

void RejectAdapterFromAnotherInstance()
{
    auto enumerated = std::make_shared<TestAdapter>(
        std::vector<Engine::Graphics::QueueType>{Engine::Graphics::QueueType::Graphics});
    auto preferred = std::make_shared<TestAdapter>(
        std::vector<Engine::Graphics::QueueType>{Engine::Graphics::QueueType::Graphics});
    auto instance = std::make_shared<TestGraphicsInstance>(
        std::vector<std::shared_ptr<Engine::Graphics::Adapter>>{enumerated});

    Engine::Error error;
    assert(!InitializeWith(instance, preferred, error));
    assert(error.message == "The selected graphics adapter was not enumerated by this instance.");
    assert(enumerated->createDeviceCalls == 0);
    assert(preferred->createDeviceCalls == 0);
}

void RejectPreferredAdapterWithoutGraphicsQueue()
{
    auto preferred = std::make_shared<TestAdapter>(
        std::vector<Engine::Graphics::QueueType>{Engine::Graphics::QueueType::Compute});
    auto instance = std::make_shared<TestGraphicsInstance>(
        std::vector<std::shared_ptr<Engine::Graphics::Adapter>>{preferred});

    Engine::Error error;
    assert(!InitializeWith(instance, preferred, error));
    assert(error.message == "The selected graphics adapter has no graphics queue.");
    assert(preferred->createDeviceCalls == 0);
}

void UsePreferredAdapterInsteadOfFirstAdapter()
{
    auto first = std::make_shared<TestAdapter>(
        std::vector<Engine::Graphics::QueueType>{Engine::Graphics::QueueType::Graphics});
    auto preferred = std::make_shared<TestAdapter>(
        std::vector<Engine::Graphics::QueueType>{Engine::Graphics::QueueType::Graphics});
    auto instance = std::make_shared<TestGraphicsInstance>(
        std::vector<std::shared_ptr<Engine::Graphics::Adapter>>{first, preferred});

    Engine::Error error;
    assert(!InitializeWith(instance, preferred, error));
    assert(error.message == "test device creation");
    assert(first->createDeviceCalls == 0);
    assert(preferred->createDeviceCalls == 1);
}

} // namespace

int main()
{
    RejectAdapterFromAnotherInstance();
    RejectPreferredAdapterWithoutGraphicsQueue();
    UsePreferredAdapterInsteadOfFirstAdapter();
}

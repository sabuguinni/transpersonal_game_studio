#include "IntegrationSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Subsystems/SubsystemBlueprintLibrary.h"

void UIntegrationSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("IntegrationSystemManager: Initializing..."));
    
    // Inicializar variáveis
    RegisteredModules.Empty();
    FailedModules.Empty();
    UnsatisfiedDependencies.Empty();
    bSystemInitialized = false;
    LastValidationTime = FDateTime::Now();
    TotalRegisteredModules = 0;
    ActiveModulesCount = 0;
    
    UE_LOG(LogTemp, Log, TEXT("IntegrationSystemManager: Initialized successfully"));
}

void UIntegrationSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("IntegrationSystemManager: Shutting down..."));
    
    // Shutdown de todos os módulos
    ShutdownAllModules();
    
    // Limpar dados
    RegisteredModules.Empty();
    FailedModules.Empty();
    UnsatisfiedDependencies.Empty();
    
    Super::Deinitialize();
}

UIntegrationSystemManager* UIntegrationSystemManager::Get(const UObject* WorldContext)
{
    if (!WorldContext)
    {
        return nullptr;
    }
    
    const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return nullptr;
    }
    
    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance)
    {
        return nullptr;
    }
    
    return GameInstance->GetSubsystem<UIntegrationSystemManager>();
}

void UIntegrationSystemManager::RegisterModule(const FString& ModuleName, UObject* ModuleInstance)
{
    if (ModuleName.IsEmpty() || !ModuleInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("IntegrationSystemManager: Cannot register module - invalid parameters"));
        return;
    }
    
    if (RegisteredModules.Contains(ModuleName))
    {
        UE_LOG(LogTemp, Warning, TEXT("IntegrationSystemManager: Module '%s' already registered"), *ModuleName);
        return;
    }
    
    // Validar o módulo antes de registar
    if (!ValidateModule(ModuleName, ModuleInstance))
    {
        FailedModules.AddUnique(ModuleName);
        UE_LOG(LogTemp, Error, TEXT("IntegrationSystemManager: Module '%s' failed validation"), *ModuleName);
        return;
    }
    
    // Registar o módulo
    RegisteredModules.Add(ModuleName, ModuleInstance);
    TotalRegisteredModules = RegisteredModules.Num();
    ActiveModulesCount++;
    
    UE_LOG(LogTemp, Log, TEXT("IntegrationSystemManager: Module '%s' registered successfully"), *ModuleName);
}

void UIntegrationSystemManager::UnregisterModule(const FString& ModuleName)
{
    if (!RegisteredModules.Contains(ModuleName))
    {
        UE_LOG(LogTemp, Warning, TEXT("IntegrationSystemManager: Module '%s' not found for unregistration"), *ModuleName);
        return;
    }
    
    RegisteredModules.Remove(ModuleName);
    TotalRegisteredModules = RegisteredModules.Num();
    ActiveModulesCount--;
    
    UE_LOG(LogTemp, Log, TEXT("IntegrationSystemManager: Module '%s' unregistered"), *ModuleName);
}

bool UIntegrationSystemManager::IsModuleRegistered(const FString& ModuleName) const
{
    return RegisteredModules.Contains(ModuleName);
}

UObject* UIntegrationSystemManager::GetModuleInstance(const FString& ModuleName) const
{
    if (const UObject* const* FoundModule = RegisteredModules.Find(ModuleName))
    {
        return *FoundModule;
    }
    
    return nullptr;
}

bool UIntegrationSystemManager::ValidateModuleDependencies()
{
    UE_LOG(LogTemp, Log, TEXT("IntegrationSystemManager: Validating module dependencies..."));
    
    UnsatisfiedDependencies.Empty();
    bool bAllDependenciesSatisfied = true;
    
    // Verificar dependências para cada módulo registado
    for (const auto& ModulePair : RegisteredModules)
    {
        const FString& ModuleName = ModulePair.Key;
        
        if (!CheckModuleDependencies(ModuleName))
        {
            bAllDependenciesSatisfied = false;
            UE_LOG(LogTemp, Warning, TEXT("IntegrationSystemManager: Module '%s' has unsatisfied dependencies"), *ModuleName);
        }
    }
    
    LastValidationTime = FDateTime::Now();
    
    UE_LOG(LogTemp, Log, TEXT("IntegrationSystemManager: Dependency validation %s"), 
           bAllDependenciesSatisfied ? TEXT("PASSED") : TEXT("FAILED"));
    
    return bAllDependenciesSatisfied;
}

FString UIntegrationSystemManager::GenerateSystemReport()
{
    FString Report;
    Report += TEXT("=== INTEGRATION SYSTEM REPORT ===\n");
    Report += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    Report += FString::Printf(TEXT("Total Registered Modules: %d\n"), TotalRegisteredModules);
    Report += FString::Printf(TEXT("Active Modules: %d\n"), ActiveModulesCount);
    Report += FString::Printf(TEXT("Failed Modules: %d\n"), FailedModules.Num());
    Report += FString::Printf(TEXT("System Healthy: %s\n"), IsSystemHealthy() ? TEXT("YES") : TEXT("NO"));
    Report += TEXT("\n");
    
    Report += TEXT("Registered Modules:\n");
    for (const auto& ModulePair : RegisteredModules)
    {
        Report += FString::Printf(TEXT("- %s: %s\n"), 
                                 *ModulePair.Key, 
                                 ModulePair.Value ? TEXT("ACTIVE") : TEXT("NULL"));
    }
    
    if (FailedModules.Num() > 0)
    {
        Report += TEXT("\nFailed Modules:\n");
        for (const FString& FailedModule : FailedModules)
        {
            Report += FString::Printf(TEXT("- %s\n"), *FailedModule);
        }
    }
    
    if (UnsatisfiedDependencies.Num() > 0)
    {
        Report += TEXT("\nUnsatisfied Dependencies:\n");
        for (const FString& Dependency : UnsatisfiedDependencies)
        {
            Report += FString::Printf(TEXT("- %s\n"), *Dependency);
        }
    }
    
    return Report;
}

bool UIntegrationSystemManager::IsSystemHealthy() const
{
    return bSystemInitialized && 
           FailedModules.Num() == 0 && 
           UnsatisfiedDependencies.Num() == 0 &&
           TotalRegisteredModules > 0;
}

void UIntegrationSystemManager::InitializeAllModules()
{
    UE_LOG(LogTemp, Log, TEXT("IntegrationSystemManager: Initializing all modules..."));
    
    // Obter ordem de inicialização baseada em dependências
    TArray<FString> InitOrder = GetModuleInitializationOrder();
    
    int32 InitializedCount = 0;
    for (const FString& ModuleName : InitOrder)
    {
        UObject* ModuleInstance = GetModuleInstance(ModuleName);
        if (ModuleInstance)
        {
            // Tentar chamar método Initialize se existir
            UFunction* InitFunction = ModuleInstance->GetClass()->FindFunctionByName(TEXT("Initialize"));
            if (InitFunction)
            {
                ModuleInstance->ProcessEvent(InitFunction, nullptr);
                InitializedCount++;
                UE_LOG(LogTemp, Log, TEXT("IntegrationSystemManager: Initialized module '%s'"), *ModuleName);
            }
        }
    }
    
    bSystemInitialized = (InitializedCount > 0);
    UE_LOG(LogTemp, Log, TEXT("IntegrationSystemManager: Initialized %d modules"), InitializedCount);
}

void UIntegrationSystemManager::ShutdownAllModules()
{
    UE_LOG(LogTemp, Log, TEXT("IntegrationSystemManager: Shutting down all modules..."));
    
    int32 ShutdownCount = 0;
    for (const auto& ModulePair : RegisteredModules)
    {
        UObject* ModuleInstance = ModulePair.Value;
        if (ModuleInstance)
        {
            // Tentar chamar método Shutdown se existir
            UFunction* ShutdownFunction = ModuleInstance->GetClass()->FindFunctionByName(TEXT("Shutdown"));
            if (ShutdownFunction)
            {
                ModuleInstance->ProcessEvent(ShutdownFunction, nullptr);
                ShutdownCount++;
                UE_LOG(LogTemp, Log, TEXT("IntegrationSystemManager: Shutdown module '%s'"), *ModulePair.Key);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("IntegrationSystemManager: Shutdown %d modules"), ShutdownCount);
}

bool UIntegrationSystemManager::ValidateModule(const FString& ModuleName, UObject* ModuleInstance)
{
    if (!ModuleInstance)
    {
        return false;
    }
    
    // Verificar se o módulo tem uma classe válida
    UClass* ModuleClass = ModuleInstance->GetClass();
    if (!ModuleClass)
    {
        return false;
    }
    
    // Verificar se o módulo não está corrompido
    if (ModuleInstance->HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed))
    {
        return false;
    }
    
    return true;
}

bool UIntegrationSystemManager::CheckModuleDependencies(const FString& ModuleName)
{
    // Definir dependências conhecidas entre módulos
    static TMap<FString, TArray<FString>> ModuleDependencies = {
        {TEXT("WorldGeneration"), {}},
        {TEXT("Environment"), {TEXT("WorldGeneration")}},
        {TEXT("Characters"), {TEXT("Environment")}},
        {TEXT("AI"), {TEXT("Characters")}},
        {TEXT("Combat"), {TEXT("AI"), TEXT("Characters")}},
        {TEXT("Audio"), {TEXT("Environment")}},
        {TEXT("VFX"), {TEXT("Environment")}},
        {TEXT("QA"), {TEXT("Combat"), TEXT("AI"), TEXT("Audio"), TEXT("VFX")}}
    };
    
    if (!ModuleDependencies.Contains(ModuleName))
    {
        // Módulo sem dependências conhecidas
        return true;
    }
    
    const TArray<FString>& Dependencies = ModuleDependencies[ModuleName];
    for (const FString& Dependency : Dependencies)
    {
        if (!IsModuleRegistered(Dependency))
        {
            UnsatisfiedDependencies.AddUnique(FString::Printf(TEXT("%s requires %s"), *ModuleName, *Dependency));
            return false;
        }
    }
    
    return true;
}

TArray<FString> UIntegrationSystemManager::GetModuleInitializationOrder()
{
    // Ordem de inicialização baseada em dependências
    TArray<FString> InitOrder = {
        TEXT("Core"),
        TEXT("Physics"),
        TEXT("WorldGeneration"),
        TEXT("Environment"),
        TEXT("Characters"),
        TEXT("Animation"),
        TEXT("AI"),
        TEXT("Combat"),
        TEXT("Audio"),
        TEXT("VFX"),
        TEXT("QA"),
        TEXT("Integration")
    };
    
    // Filtrar apenas módulos que estão registados
    TArray<FString> FilteredOrder;
    for (const FString& ModuleName : InitOrder)
    {
        if (IsModuleRegistered(ModuleName))
        {
            FilteredOrder.Add(ModuleName);
        }
    }
    
    return FilteredOrder;
}
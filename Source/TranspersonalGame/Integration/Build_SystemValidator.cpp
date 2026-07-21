#include "Build_SystemValidator.h"
#include "Build_IntegrationReport.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"

UBuild_SystemValidator::UBuild_SystemValidator()
{
    CurrentReport = nullptr;
}

void UBuild_SystemValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Build_SystemValidator: Initializing system validator"));
    
    // Register all core systems
    RegisterCoreSystem();
    RegisterWorldSystems();
    RegisterCharacterSystems();
    RegisterAISystems();
    
    // Create initial report
    CurrentReport = NewObject<UBuild_IntegrationReport>(this);
    
    UE_LOG(LogTemp, Log, TEXT("Build_SystemValidator: Initialization complete - %d systems registered"), RegisteredSystems.Num());
}

void UBuild_SystemValidator::Deinitialize()
{
    RegisteredSystems.Empty();
    FailedValidations.Empty();
    MissingDependencies.Empty();
    CurrentReport = nullptr;
    
    Super::Deinitialize();
}

void UBuild_SystemValidator::RegisterSystem(const FBuild_SystemInfo& SystemInfo)
{
    // Check if system is already registered
    for (FBuild_SystemInfo& ExistingSystem : RegisteredSystems)
    {
        if (ExistingSystem.SystemName == SystemInfo.SystemName)
        {
            // Update existing system
            ExistingSystem = SystemInfo;
            UE_LOG(LogTemp, Log, TEXT("Build_SystemValidator: Updated system %s"), *SystemInfo.SystemName);
            return;
        }
    }
    
    // Add new system
    RegisteredSystems.Add(SystemInfo);
    UE_LOG(LogTemp, Log, TEXT("Build_SystemValidator: Registered new system %s"), *SystemInfo.SystemName);
}

bool UBuild_SystemValidator::ValidateSystem(const FString& SystemName)
{
    for (FBuild_SystemInfo& SystemInfo : RegisteredSystems)
    {
        if (SystemInfo.SystemName == SystemName)
        {
            if (!SystemInfo.bIsEnabled)
            {
                UE_LOG(LogTemp, Warning, TEXT("Build_SystemValidator: System %s is disabled"), *SystemName);
                return false;
            }
            
            bool bClassesValid = ValidateSystemClasses(SystemInfo);
            bool bDependenciesValid = ValidateSystemDependencies(SystemInfo);
            
            SystemInfo.bIsValidated = bClassesValid && bDependenciesValid;
            
            if (!SystemInfo.bIsValidated)
            {
                FailedValidations.AddUnique(SystemName);
            }
            else
            {
                FailedValidations.Remove(SystemName);
            }
            
            return SystemInfo.bIsValidated;
        }
    }
    
    UE_LOG(LogTemp, Error, TEXT("Build_SystemValidator: System %s not found"), *SystemName);
    return false;
}

bool UBuild_SystemValidator::ValidateAllSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Build_SystemValidator: Starting validation of all systems"));
    
    FailedValidations.Empty();
    MissingDependencies.Empty();
    
    bool bAllValid = true;
    
    for (FBuild_SystemInfo& SystemInfo : RegisteredSystems)
    {
        if (SystemInfo.bIsEnabled)
        {
            bool bSystemValid = ValidateSystem(SystemInfo.SystemName);
            if (!bSystemValid)
            {
                bAllValid = false;
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Build_SystemValidator: Validation complete - All systems valid: %s"), 
           bAllValid ? TEXT("true") : TEXT("false"));
    
    return bAllValid;
}

UBuild_IntegrationReport* UBuild_SystemValidator::GenerateFullReport()
{
    if (!CurrentReport)
    {
        CurrentReport = NewObject<UBuild_IntegrationReport>(this);
    }
    
    CurrentReport->CycleID = TEXT("PROD_CYCLE_AUTO_20260601_008");
    CurrentReport->GenerateReport();
    
    // Add system-specific information
    for (const FBuild_SystemInfo& SystemInfo : RegisteredSystems)
    {
        if (SystemInfo.bIsEnabled && !SystemInfo.bIsValidated)
        {
            CurrentReport->AddCriticalError(FString::Printf(TEXT("System validation failed: %s"), *SystemInfo.SystemName));
        }
    }
    
    for (const FString& MissingDep : MissingDependencies)
    {
        CurrentReport->AddWarning(FString::Printf(TEXT("Missing dependency: %s"), *MissingDep));
    }
    
    return CurrentReport;
}

TArray<FString> UBuild_SystemValidator::GetFailedSystems()
{
    return FailedValidations;
}

TArray<FString> UBuild_SystemValidator::GetMissingDependencies()
{
    return MissingDependencies;
}

bool UBuild_SystemValidator::IsSystemRegistered(const FString& SystemName)
{
    for (const FBuild_SystemInfo& SystemInfo : RegisteredSystems)
    {
        if (SystemInfo.SystemName == SystemName)
        {
            return true;
        }
    }
    return false;
}

FBuild_SystemInfo UBuild_SystemValidator::GetSystemInfo(const FString& SystemName)
{
    for (const FBuild_SystemInfo& SystemInfo : RegisteredSystems)
    {
        if (SystemInfo.SystemName == SystemName)
        {
            return SystemInfo;
        }
    }
    
    // Return default if not found
    FBuild_SystemInfo DefaultInfo;
    DefaultInfo.SystemName = SystemName;
    DefaultInfo.bIsEnabled = false;
    DefaultInfo.bIsValidated = false;
    return DefaultInfo;
}

void UBuild_SystemValidator::EnableSystem(const FString& SystemName, bool bEnable)
{
    for (FBuild_SystemInfo& SystemInfo : RegisteredSystems)
    {
        if (SystemInfo.SystemName == SystemName)
        {
            SystemInfo.bIsEnabled = bEnable;
            if (!bEnable)
            {
                SystemInfo.bIsValidated = false;
                FailedValidations.Remove(SystemName);
            }
            UE_LOG(LogTemp, Log, TEXT("Build_SystemValidator: System %s %s"), 
                   *SystemName, bEnable ? TEXT("enabled") : TEXT("disabled"));
            return;
        }
    }
}

int32 UBuild_SystemValidator::GetValidatedSystemCount()
{
    int32 Count = 0;
    for (const FBuild_SystemInfo& SystemInfo : RegisteredSystems)
    {
        if (SystemInfo.bIsEnabled && SystemInfo.bIsValidated)
        {
            Count++;
        }
    }
    return Count;
}

float UBuild_SystemValidator::GetSystemValidationProgress()
{
    int32 EnabledSystems = 0;
    int32 ValidatedSystems = 0;
    
    for (const FBuild_SystemInfo& SystemInfo : RegisteredSystems)
    {
        if (SystemInfo.bIsEnabled)
        {
            EnabledSystems++;
            if (SystemInfo.bIsValidated)
            {
                ValidatedSystems++;
            }
        }
    }
    
    return EnabledSystems > 0 ? (float)ValidatedSystems / (float)EnabledSystems : 0.0f;
}

void UBuild_SystemValidator::RegisterCoreSystem()
{
    FBuild_SystemInfo CoreSystem;
    CoreSystem.SystemName = TEXT("Core");
    CoreSystem.SystemType = EBuild_SystemType::Core;
    CoreSystem.RequiredClasses = {
        TEXT("TranspersonalGameState"),
        TEXT("TranspersonalCharacter"),
        TEXT("BuildIntegrationManager")
    };
    CoreSystem.bIsEnabled = true;
    
    RegisterSystem(CoreSystem);
}

void UBuild_SystemValidator::RegisterWorldSystems()
{
    FBuild_SystemInfo WorldSystem;
    WorldSystem.SystemName = TEXT("WorldGeneration");
    WorldSystem.SystemType = EBuild_SystemType::World;
    WorldSystem.RequiredClasses = {
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager"),
        TEXT("ProceduralWorldManager")
    };
    WorldSystem.Dependencies = { TEXT("Core") };
    WorldSystem.bIsEnabled = true;
    
    RegisterSystem(WorldSystem);
}

void UBuild_SystemValidator::RegisterCharacterSystems()
{
    FBuild_SystemInfo CharacterSystem;
    CharacterSystem.SystemName = TEXT("Character");
    CharacterSystem.SystemType = EBuild_SystemType::Character;
    CharacterSystem.RequiredClasses = {
        TEXT("TranspersonalCharacter")
    };
    CharacterSystem.Dependencies = { TEXT("Core") };
    CharacterSystem.bIsEnabled = true;
    
    RegisterSystem(CharacterSystem);
}

void UBuild_SystemValidator::RegisterAISystems()
{
    FBuild_SystemInfo AISystem;
    AISystem.SystemName = TEXT("AI");
    AISystem.SystemType = EBuild_SystemType::AI;
    AISystem.RequiredClasses = {
        TEXT("CrowdSimulationManager")
    };
    AISystem.Dependencies = { TEXT("Core"), TEXT("Character") };
    AISystem.bIsEnabled = true;
    
    RegisterSystem(AISystem);
}

bool UBuild_SystemValidator::ValidateSystemClasses(const FBuild_SystemInfo& SystemInfo)
{
    bool bAllClassesValid = true;
    
    for (const FString& ClassName : SystemInfo.RequiredClasses)
    {
        if (!CheckClassExists(ClassName))
        {
            UE_LOG(LogTemp, Error, TEXT("Build_SystemValidator: Required class not found: %s"), *ClassName);
            bAllClassesValid = false;
        }
    }
    
    for (const FString& ClassName : SystemInfo.OptionalClasses)
    {
        if (!CheckClassExists(ClassName))
        {
            UE_LOG(LogTemp, Warning, TEXT("Build_SystemValidator: Optional class not found: %s"), *ClassName);
        }
    }
    
    return bAllClassesValid;
}

bool UBuild_SystemValidator::ValidateSystemDependencies(const FBuild_SystemInfo& SystemInfo)
{
    bool bAllDependenciesValid = true;
    
    for (const FString& Dependency : SystemInfo.Dependencies)
    {
        bool bDependencyFound = false;
        for (const FBuild_SystemInfo& OtherSystem : RegisteredSystems)
        {
            if (OtherSystem.SystemName == Dependency && OtherSystem.bIsEnabled && OtherSystem.bIsValidated)
            {
                bDependencyFound = true;
                break;
            }
        }
        
        if (!bDependencyFound)
        {
            UE_LOG(LogTemp, Error, TEXT("Build_SystemValidator: Dependency not satisfied: %s"), *Dependency);
            MissingDependencies.AddUnique(Dependency);
            bAllDependenciesValid = false;
        }
    }
    
    return bAllDependenciesValid;
}

bool UBuild_SystemValidator::CheckClassExists(const FString& ClassName)
{
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    UClass* TestClass = LoadClass<UObject>(nullptr, *ClassPath);
    return TestClass != nullptr;
}
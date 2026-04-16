#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManager.h"

DEFINE_LOG_CATEGORY(LogBuildIntegration);

UBuildIntegrationManager::UBuildIntegrationManager()
{
    TargetFrameRate = 60.0f;
    MinimumFrameRate = 30.0f;
    bLastBuildSuccessful = false;
    LastBuildTimestamp = TEXT("");

    // Initialize known modules
    KnownModules.Add(TEXT("TranspersonalGame"));
    KnownModules.Add(TEXT("Engine"));
    KnownModules.Add(TEXT("Core"));
    KnownModules.Add(TEXT("CoreUObject"));

    // Initialize critical assets
    CriticalAssets.Add(TEXT("/Game/Maps/MinPlayableMap"));
    CriticalAssets.Add(TEXT("/Game/TranspersonalGame/Characters/TranspersonalCharacter"));
    CriticalAssets.Add(TEXT("/Game/TranspersonalGame/GameModes/TranspersonalGameMode"));
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogBuildIntegration, Log, TEXT("BuildIntegrationManager initialized"));
    
    // Run initial integration test
    LastIntegrationReport = RunFullIntegrationTest();
    
    // Set build timestamp
    LastBuildTimestamp = FDateTime::Now().ToString();
}

void UBuildIntegrationManager::Deinitialize()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("BuildIntegrationManager deinitialized"));
    Super::Deinitialize();
}

FBuild_IntegrationReport UBuildIntegrationManager::RunFullIntegrationTest()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Running full integration test"));
    
    FBuild_IntegrationReport Report;
    Report.BuildTimestamp = FDateTime::Now().ToString();
    
    // Test each known module
    for (const FString& ModuleName : KnownModules)
    {
        FBuild_ModuleStatus ModuleStatus;
        ModuleStatus.ModuleName = ModuleName;
        ModuleStatus.bIsLoaded = ValidateModuleIntegration(ModuleName);
        
        if (ModuleStatus.bIsLoaded)
        {
            ModuleStatus.ClassCount = 1; // Simplified for now
            Report.LoadedClasses++;
        }
        else
        {
            ModuleStatus.bHasErrors = true;
            ModuleStatus.ErrorMessage = FString::Printf(TEXT("Module %s failed to load"), *ModuleName);
            Report.FailedClasses++;
        }
        
        Report.ModuleStatuses.Add(ModuleStatus);
        Report.TotalClasses++;
    }
    
    // Test cross-system integration
    bool bCrossSystemSuccess = TestCrossSystemIntegration();
    
    // Test asset validation
    bool bAssetsValid = ValidateGameAssets();
    
    // Test performance targets
    bool bPerformanceValid = ValidatePerformanceTargets();
    
    // Determine overall build success
    Report.bBuildSuccessful = (Report.FailedClasses == 0) && bCrossSystemSuccess && bAssetsValid && bPerformanceValid;
    bLastBuildSuccessful = Report.bBuildSuccessful;
    
    // Log results
    LogIntegrationResults(Report);
    
    // Store report
    LastIntegrationReport = Report;
    
    return Report;
}

bool UBuildIntegrationManager::ValidateModuleIntegration(const FString& ModuleName)
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Validating module: %s"), *ModuleName);
    
    // Try to load a representative class from each module
    if (ModuleName == TEXT("TranspersonalGame"))
    {
        return ValidateModuleClasses(ModuleName);
    }
    else if (ModuleName == TEXT("Engine"))
    {
        // Engine module is always loaded if we're running
        return true;
    }
    else if (ModuleName == TEXT("Core") || ModuleName == TEXT("CoreUObject"))
    {
        // Core modules are always loaded
        return true;
    }
    
    return false;
}

bool UBuildIntegrationManager::ValidateModuleClasses(const FString& ModuleName)
{
    // Test loading of key TranspersonalGame classes
    TArray<FString> TestClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.BuildIntegrationManager")
    };
    
    int32 LoadedCount = 0;
    for (const FString& ClassName : TestClasses)
    {
        UClass* TestClass = LoadClass<UObject>(nullptr, *ClassName);
        if (TestClass)
        {
            LoadedCount++;
            UE_LOG(LogBuildIntegration, Log, TEXT("Successfully loaded class: %s"), *ClassName);
        }
        else
        {
            UE_LOG(LogBuildIntegration, Warning, TEXT("Failed to load class: %s"), *ClassName);
        }
    }
    
    return LoadedCount > 0; // At least some classes should load
}

TArray<FString> UBuildIntegrationManager::GetLoadedModules()
{
    TArray<FString> LoadedModules;
    
    for (const FString& ModuleName : KnownModules)
    {
        if (ValidateModuleIntegration(ModuleName))
        {
            LoadedModules.Add(ModuleName);
        }
    }
    
    return LoadedModules;
}

bool UBuildIntegrationManager::TestCrossSystemIntegration()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Testing cross-system integration"));
    
    // Test basic system interactions
    bool bSuccess = true;
    
    // Test 1: World and Character systems
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogBuildIntegration, Error, TEXT("No world available for integration test"));
        bSuccess = false;
    }
    
    // Test 2: Game Instance availability
    UGameInstance* GameInstance = GetGameInstance();
    if (!GameInstance)
    {
        UE_LOG(LogBuildIntegration, Error, TEXT("No game instance available"));
        bSuccess = false;
    }
    
    // Test 3: Asset Registry
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    if (!AssetRegistryModule.Get().IsLoadingAssets())
    {
        UE_LOG(LogBuildIntegration, Log, TEXT("Asset registry is ready"));
    }
    
    return bSuccess;
}

bool UBuildIntegrationManager::ValidateGameAssets()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Validating game assets"));
    
    bool bAllAssetsValid = true;
    
    for (const FString& AssetPath : CriticalAssets)
    {
        // Try to load each critical asset
        UObject* Asset = LoadObject<UObject>(nullptr, *AssetPath);
        if (Asset)
        {
            UE_LOG(LogBuildIntegration, Log, TEXT("Asset validated: %s"), *AssetPath);
        }
        else
        {
            UE_LOG(LogBuildIntegration, Warning, TEXT("Asset not found: %s"), *AssetPath);
            bAllAssetsValid = false;
        }
    }
    
    return bAllAssetsValid;
}

TArray<FString> UBuildIntegrationManager::GetMissingAssets()
{
    TArray<FString> MissingAssets;
    
    for (const FString& AssetPath : CriticalAssets)
    {
        UObject* Asset = LoadObject<UObject>(nullptr, *AssetPath);
        if (!Asset)
        {
            MissingAssets.Add(AssetPath);
        }
    }
    
    return MissingAssets;
}

bool UBuildIntegrationManager::ValidatePerformanceTargets()
{
    float CurrentFPS = GetCurrentFrameRate();
    
    bool bPerformanceValid = CurrentFPS >= MinimumFrameRate;
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Performance validation - Current FPS: %.2f, Target: %.2f, Minimum: %.2f"), 
           CurrentFPS, TargetFrameRate, MinimumFrameRate);
    
    return bPerformanceValid;
}

float UBuildIntegrationManager::GetCurrentFrameRate()
{
    // Get current frame rate from engine
    if (GEngine && GEngine->GetGameViewport())
    {
        return 1.0f / FApp::GetDeltaTime();
    }
    
    return 0.0f;
}

bool UBuildIntegrationManager::IsProjectCompiled()
{
    // Check if the project appears to be compiled by testing module loading
    return ValidateModuleIntegration(TEXT("TranspersonalGame"));
}

FString UBuildIntegrationManager::GetLastCompilationError()
{
    // Return the last known compilation error
    if (!bLastBuildSuccessful)
    {
        return TEXT("Module loading failures detected - check log for details");
    }
    
    return TEXT("No compilation errors detected");
}

void UBuildIntegrationManager::GenerateBuildReport()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("=== BUILD INTEGRATION REPORT ==="));
    UE_LOG(LogBuildIntegration, Log, TEXT("Timestamp: %s"), *LastBuildTimestamp);
    UE_LOG(LogBuildIntegration, Log, TEXT("Build Successful: %s"), bLastBuildSuccessful ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogBuildIntegration, Log, TEXT("Total Classes: %d"), LastIntegrationReport.TotalClasses);
    UE_LOG(LogBuildIntegration, Log, TEXT("Loaded Classes: %d"), LastIntegrationReport.LoadedClasses);
    UE_LOG(LogBuildIntegration, Log, TEXT("Failed Classes: %d"), LastIntegrationReport.FailedClasses);
    
    for (const FBuild_ModuleStatus& ModuleStatus : LastIntegrationReport.ModuleStatuses)
    {
        UE_LOG(LogBuildIntegration, Log, TEXT("Module %s: %s"), 
               *ModuleStatus.ModuleName, 
               ModuleStatus.bIsLoaded ? TEXT("LOADED") : TEXT("FAILED"));
    }
    
    UE_LOG(LogBuildIntegration, Log, TEXT("=== END BUILD REPORT ==="));
}

bool UBuildIntegrationManager::TestSystemInteractions()
{
    // Test interactions between different game systems
    UE_LOG(LogBuildIntegration, Log, TEXT("Testing system interactions"));
    
    // This would test actual cross-system functionality
    // For now, return true as a placeholder
    return true;
}

void UBuildIntegrationManager::LogIntegrationResults(const FBuild_IntegrationReport& Report)
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Integration test completed"));
    UE_LOG(LogBuildIntegration, Log, TEXT("Build successful: %s"), Report.bBuildSuccessful ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogBuildIntegration, Log, TEXT("Modules tested: %d"), Report.ModuleStatuses.Num());
    UE_LOG(LogBuildIntegration, Log, TEXT("Classes loaded: %d/%d"), Report.LoadedClasses, Report.TotalClasses);
}
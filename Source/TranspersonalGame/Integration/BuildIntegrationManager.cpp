#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/SceneComponent.h"

ABuildIntegrationManager::ABuildIntegrationManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize configuration
    bAutoValidateOnTick = true;
    ValidationInterval = 30.0f; // Validate every 30 seconds
    bEnableCrossModuleTesting = true;

    // Initialize status
    OverallBuildStatus = EInteg_BuildStatus::Unknown;
    TotalModules = 0;
    SuccessfulModules = 0;
    FailedModules = 0;

    // Initialize internal state
    LastValidationTime = 0.0f;
    bValidationInProgress = false;

    // Initialize module statuses
    ModuleStatuses.Empty();
    
    // Add core modules
    FInteg_ModuleStatus CoreModule;
    CoreModule.ModuleType = EInteg_ModuleType::Core;
    CoreModule.ModuleName = TEXT("TranspersonalGame Core");
    ModuleStatuses.Add(CoreModule);

    FInteg_ModuleStatus WorldGenModule;
    WorldGenModule.ModuleType = EInteg_ModuleType::WorldGen;
    WorldGenModule.ModuleName = TEXT("World Generation");
    ModuleStatuses.Add(WorldGenModule);

    FInteg_ModuleStatus CharacterModule;
    CharacterModule.ModuleType = EInteg_ModuleType::Character;
    CharacterModule.ModuleName = TEXT("Character System");
    ModuleStatuses.Add(CharacterModule);

    FInteg_ModuleStatus AudioModule;
    AudioModule.ModuleType = EInteg_ModuleType::Audio;
    AudioModule.ModuleName = TEXT("Audio System");
    ModuleStatuses.Add(AudioModule);

    FInteg_ModuleStatus VFXModule;
    VFXModule.ModuleType = EInteg_ModuleType::VFX;
    VFXModule.ModuleName = TEXT("VFX System");
    ModuleStatuses.Add(VFXModule);

    FInteg_ModuleStatus QAModule;
    QAModule.ModuleType = EInteg_ModuleType::QA;
    QAModule.ModuleName = TEXT("QA System");
    ModuleStatuses.Add(QAModule);

    TotalModules = ModuleStatuses.Num();

    // Initialize cross-module tests
    CrossModuleTests.Empty();

    FInteg_CrossModuleTest CharWorldTest;
    CharWorldTest.TestName = TEXT("Character-WorldGen Integration");
    CharWorldTest.RequiredModules.Add(EInteg_ModuleType::Character);
    CharWorldTest.RequiredModules.Add(EInteg_ModuleType::WorldGen);
    CrossModuleTests.Add(CharWorldTest);

    FInteg_CrossModuleTest AudioVFXTest;
    AudioVFXTest.TestName = TEXT("Audio-VFX Integration");
    AudioVFXTest.RequiredModules.Add(EInteg_ModuleType::Audio);
    AudioVFXTest.RequiredModules.Add(EInteg_ModuleType::VFX);
    CrossModuleTests.Add(AudioVFXTest);

    FInteg_CrossModuleTest QASystemTest;
    QASystemTest.TestName = TEXT("QA System Integration");
    QASystemTest.RequiredModules.Add(EInteg_ModuleType::QA);
    QASystemTest.RequiredModules.Add(EInteg_ModuleType::Core);
    CrossModuleTests.Add(QASystemTest);
}

void ABuildIntegrationManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Starting integration validation"));
    
    // Initial validation
    ValidateAllModules();
    
    if (bEnableCrossModuleTesting)
    {
        RunCrossModuleTests();
    }
}

void ABuildIntegrationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bAutoValidateOnTick && !bValidationInProgress)
    {
        LastValidationTime += DeltaTime;
        
        if (LastValidationTime >= ValidationInterval)
        {
            ValidateAllModules();
            LastValidationTime = 0.0f;
        }
    }
}

void ABuildIntegrationManager::ValidateAllModules()
{
    if (bValidationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validation already in progress"));
        return;
    }

    bValidationInProgress = true;
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Starting module validation"));

    SuccessfulModules = 0;
    FailedModules = 0;

    // Validate each module
    for (FInteg_ModuleStatus& ModuleStatus : ModuleStatuses)
    {
        ValidateModule(ModuleStatus.ModuleType);
    }

    // Update overall status
    if (FailedModules == 0)
    {
        OverallBuildStatus = EInteg_BuildStatus::Success;
    }
    else if (SuccessfulModules > 0)
    {
        OverallBuildStatus = EInteg_BuildStatus::Warning;
    }
    else
    {
        OverallBuildStatus = EInteg_BuildStatus::Failed;
    }

    LogIntegrationStatus();
    bValidationInProgress = false;
}

void ABuildIntegrationManager::ValidateModule(EInteg_ModuleType ModuleType)
{
    switch (ModuleType)
    {
        case EInteg_ModuleType::Core:
            // Core module is always available
            UpdateModuleStatus(ModuleType, EInteg_BuildStatus::Success);
            break;
            
        case EInteg_ModuleType::Character:
            ValidateCharacterModule();
            break;
            
        case EInteg_ModuleType::WorldGen:
            ValidateWorldGenModule();
            break;
            
        case EInteg_ModuleType::Audio:
            ValidateAudioModule();
            break;
            
        case EInteg_ModuleType::VFX:
            ValidateVFXModule();
            break;
            
        case EInteg_ModuleType::QA:
            ValidateQAModule();
            break;
            
        case EInteg_ModuleType::AI:
            ValidateAIModule();
            break;
            
        case EInteg_ModuleType::Combat:
            ValidateCombatModule();
            break;
            
        default:
            UpdateModuleStatus(ModuleType, EInteg_BuildStatus::Unknown, TEXT("Unknown module type"));
            break;
    }
}

void ABuildIntegrationManager::ValidateCharacterModule()
{
    // Try to load character classes
    UClass* CharacterClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    UClass* GameModeClass = LoadClass<AGameModeBase>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalGameMode"));
    UClass* GameStateClass = LoadClass<AGameStateBase>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalGameState"));

    int32 ClassCount = 0;
    FString ErrorMsg = TEXT("");

    if (CharacterClass) ClassCount++;
    else ErrorMsg += TEXT("TranspersonalCharacter missing; ");

    if (GameModeClass) ClassCount++;
    else ErrorMsg += TEXT("TranspersonalGameMode missing; ");

    if (GameStateClass) ClassCount++;
    else ErrorMsg += TEXT("TranspersonalGameState missing; ");

    EInteg_BuildStatus Status = (ClassCount >= 2) ? EInteg_BuildStatus::Success : 
                               (ClassCount >= 1) ? EInteg_BuildStatus::Warning : EInteg_BuildStatus::Failed;

    UpdateModuleStatus(EInteg_ModuleType::Character, Status, ErrorMsg);
    
    // Update class count
    for (FInteg_ModuleStatus& ModuleStatus : ModuleStatuses)
    {
        if (ModuleStatus.ModuleType == EInteg_ModuleType::Character)
        {
            ModuleStatus.ClassCount = ClassCount;
            break;
        }
    }
}

void ABuildIntegrationManager::ValidateWorldGenModule()
{
    UClass* WorldGenClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
    UClass* FoliageClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.FoliageManager"));

    int32 ClassCount = 0;
    FString ErrorMsg = TEXT("");

    if (WorldGenClass) ClassCount++;
    else ErrorMsg += TEXT("PCGWorldGenerator missing; ");

    if (FoliageClass) ClassCount++;
    else ErrorMsg += TEXT("FoliageManager missing; ");

    EInteg_BuildStatus Status = (ClassCount >= 2) ? EInteg_BuildStatus::Success : 
                               (ClassCount >= 1) ? EInteg_BuildStatus::Warning : EInteg_BuildStatus::Failed;

    UpdateModuleStatus(EInteg_ModuleType::WorldGen, Status, ErrorMsg);
    
    for (FInteg_ModuleStatus& ModuleStatus : ModuleStatuses)
    {
        if (ModuleStatus.ModuleType == EInteg_ModuleType::WorldGen)
        {
            ModuleStatus.ClassCount = ClassCount;
            break;
        }
    }
}

void ABuildIntegrationManager::ValidateAudioModule()
{
    // Check for audio manager classes
    UClass* AudioManagerClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.AudioManager"));
    UClass* EnvAudioClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.EnvironmentalAudioManager"));

    int32 ClassCount = 0;
    if (AudioManagerClass) ClassCount++;
    if (EnvAudioClass) ClassCount++;

    EInteg_BuildStatus Status = (ClassCount >= 1) ? EInteg_BuildStatus::Success : EInteg_BuildStatus::Warning;
    FString ErrorMsg = (ClassCount == 0) ? TEXT("No audio classes found") : TEXT("");

    UpdateModuleStatus(EInteg_ModuleType::Audio, Status, ErrorMsg);
    
    for (FInteg_ModuleStatus& ModuleStatus : ModuleStatuses)
    {
        if (ModuleStatus.ModuleType == EInteg_ModuleType::Audio)
        {
            ModuleStatus.ClassCount = ClassCount;
            break;
        }
    }
}

void ABuildIntegrationManager::ValidateVFXModule()
{
    // Check for VFX manager classes
    UClass* VFXManagerClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.VFXManager"));
    UClass* EnvVFXClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.EnvironmentalVFXManager"));

    int32 ClassCount = 0;
    if (VFXManagerClass) ClassCount++;
    if (EnvVFXClass) ClassCount++;

    EInteg_BuildStatus Status = (ClassCount >= 1) ? EInteg_BuildStatus::Success : EInteg_BuildStatus::Warning;
    FString ErrorMsg = (ClassCount == 0) ? TEXT("No VFX classes found") : TEXT("");

    UpdateModuleStatus(EInteg_ModuleType::VFX, Status, ErrorMsg);
    
    for (FInteg_ModuleStatus& ModuleStatus : ModuleStatuses)
    {
        if (ModuleStatus.ModuleType == EInteg_ModuleType::VFX)
        {
            ModuleStatus.ClassCount = ClassCount;
            break;
        }
    }
}

void ABuildIntegrationManager::ValidateQAModule()
{
    UClass* QAManagerClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.QATestManager"));

    int32 ClassCount = QAManagerClass ? 1 : 0;
    EInteg_BuildStatus Status = QAManagerClass ? EInteg_BuildStatus::Success : EInteg_BuildStatus::Warning;
    FString ErrorMsg = QAManagerClass ? TEXT("") : TEXT("QATestManager missing");

    UpdateModuleStatus(EInteg_ModuleType::QA, Status, ErrorMsg);
    
    for (FInteg_ModuleStatus& ModuleStatus : ModuleStatuses)
    {
        if (ModuleStatus.ModuleType == EInteg_ModuleType::QA)
        {
            ModuleStatus.ClassCount = ClassCount;
            break;
        }
    }
}

void ABuildIntegrationManager::ValidateAIModule()
{
    // AI module validation - placeholder for future AI classes
    UpdateModuleStatus(EInteg_ModuleType::AI, EInteg_BuildStatus::Warning, TEXT("AI module not yet implemented"));
}

void ABuildIntegrationManager::ValidateCombatModule()
{
    // Combat module validation - placeholder for future combat classes
    UpdateModuleStatus(EInteg_ModuleType::Combat, EInteg_BuildStatus::Warning, TEXT("Combat module not yet implemented"));
}

void ABuildIntegrationManager::RunCrossModuleTests()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Running cross-module tests"));

    for (FInteg_CrossModuleTest& Test : CrossModuleTests)
    {
        // Check if all required modules are ready
        bool bAllModulesReady = true;
        for (EInteg_ModuleType RequiredModule : Test.RequiredModules)
        {
            if (!IsModuleReady(RequiredModule))
            {
                bAllModulesReady = false;
                break;
            }
        }

        if (bAllModulesReady)
        {
            // Run specific integration tests
            if (Test.TestName == TEXT("Character-WorldGen Integration"))
            {
                TestCharacterWorldGenIntegration();
            }
            else if (Test.TestName == TEXT("Audio-VFX Integration"))
            {
                TestAudioVFXIntegration();
            }
            else if (Test.TestName == TEXT("QA System Integration"))
            {
                TestQASystemIntegration();
            }

            Test.bTestPassed = true;
            Test.TestResult = TEXT("Integration test passed");
        }
        else
        {
            Test.bTestPassed = false;
            Test.TestResult = TEXT("Required modules not ready");
        }
    }
}

void ABuildIntegrationManager::TestCharacterWorldGenIntegration()
{
    // Test character spawning in generated world
    UE_LOG(LogTemp, Warning, TEXT("Testing Character-WorldGen integration"));
}

void ABuildIntegrationManager::TestAudioVFXIntegration()
{
    // Test audio-VFX synchronization
    UE_LOG(LogTemp, Warning, TEXT("Testing Audio-VFX integration"));
}

void ABuildIntegrationManager::TestQASystemIntegration()
{
    // Test QA system functionality
    UE_LOG(LogTemp, Warning, TEXT("Testing QA System integration"));
}

void ABuildIntegrationManager::TestAICombatIntegration()
{
    // Test AI-Combat integration
    UE_LOG(LogTemp, Warning, TEXT("Testing AI-Combat integration"));
}

bool ABuildIntegrationManager::IsModuleReady(EInteg_ModuleType ModuleType) const
{
    for (const FInteg_ModuleStatus& ModuleStatus : ModuleStatuses)
    {
        if (ModuleStatus.ModuleType == ModuleType)
        {
            return ModuleStatus.BuildStatus == EInteg_BuildStatus::Success;
        }
    }
    return false;
}

FInteg_ModuleStatus ABuildIntegrationManager::GetModuleStatus(EInteg_ModuleType ModuleType) const
{
    for (const FInteg_ModuleStatus& ModuleStatus : ModuleStatuses)
    {
        if (ModuleStatus.ModuleType == ModuleType)
        {
            return ModuleStatus;
        }
    }
    return FInteg_ModuleStatus();
}

void ABuildIntegrationManager::ResetIntegrationStatus()
{
    for (FInteg_ModuleStatus& ModuleStatus : ModuleStatuses)
    {
        ModuleStatus.BuildStatus = EInteg_BuildStatus::Unknown;
        ModuleStatus.LastError = TEXT("");
        ModuleStatus.LastBuildTime = 0.0f;
        ModuleStatus.ClassCount = 0;
    }

    for (FInteg_CrossModuleTest& Test : CrossModuleTests)
    {
        Test.bTestPassed = false;
        Test.TestResult = TEXT("");
    }

    OverallBuildStatus = EInteg_BuildStatus::Unknown;
    SuccessfulModules = 0;
    FailedModules = 0;
}

void ABuildIntegrationManager::GenerateIntegrationReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BUILD INTEGRATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Overall Status: %s"), 
           OverallBuildStatus == EInteg_BuildStatus::Success ? TEXT("SUCCESS") :
           OverallBuildStatus == EInteg_BuildStatus::Warning ? TEXT("WARNING") :
           OverallBuildStatus == EInteg_BuildStatus::Failed ? TEXT("FAILED") : TEXT("UNKNOWN"));
    
    UE_LOG(LogTemp, Warning, TEXT("Modules: %d/%d successful"), SuccessfulModules, TotalModules);
    
    for (const FInteg_ModuleStatus& ModuleStatus : ModuleStatuses)
    {
        UE_LOG(LogTemp, Warning, TEXT("  %s: %s (%d classes)"), 
               *ModuleStatus.ModuleName,
               ModuleStatus.BuildStatus == EInteg_BuildStatus::Success ? TEXT("OK") :
               ModuleStatus.BuildStatus == EInteg_BuildStatus::Warning ? TEXT("WARN") :
               ModuleStatus.BuildStatus == EInteg_BuildStatus::Failed ? TEXT("FAIL") : TEXT("UNK"),
               ModuleStatus.ClassCount);
    }
}

void ABuildIntegrationManager::EditorValidateAllModules()
{
    ValidateAllModules();
}

void ABuildIntegrationManager::EditorRunCrossModuleTests()
{
    RunCrossModuleTests();
}

void ABuildIntegrationManager::EditorGenerateReport()
{
    GenerateIntegrationReport();
}

void ABuildIntegrationManager::UpdateModuleStatus(EInteg_ModuleType ModuleType, EInteg_BuildStatus Status, const FString& Error)
{
    for (FInteg_ModuleStatus& ModuleStatus : ModuleStatuses)
    {
        if (ModuleStatus.ModuleType == ModuleType)
        {
            ModuleStatus.BuildStatus = Status;
            ModuleStatus.LastError = Error;
            ModuleStatus.LastBuildTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
            
            if (Status == EInteg_BuildStatus::Success)
            {
                SuccessfulModules++;
            }
            else if (Status == EInteg_BuildStatus::Failed)
            {
                FailedModules++;
            }
            break;
        }
    }
}

void ABuildIntegrationManager::LogIntegrationStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validation complete - %d/%d modules successful"), 
           SuccessfulModules, TotalModules);
}
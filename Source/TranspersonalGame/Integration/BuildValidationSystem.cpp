#include "BuildValidationSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

ABuildValidationSystem::ABuildValidationSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize validation settings
    ValidationInterval = 30.0f; // Validate every 30 seconds
    bAutoValidation = true;
    OverallStatus = EBuild_ValidationStatus::Unknown;
    TotalErrors = 0;
    TotalWarnings = 0;
    LastValidationTime = 0.0f;
    bValidationInProgress = false;

    // Initialize module list
    InitializeModuleList();
}

void ABuildValidationSystem::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("BuildValidationSystem: Starting build validation system"));
    
    // Start initial validation
    if (bAutoValidation)
    {
        StartFullValidation();
    }
}

void ABuildValidationSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Auto-validation timer
    if (bAutoValidation && !bValidationInProgress)
    {
        LastValidationTime += DeltaTime;
        if (LastValidationTime >= ValidationInterval)
        {
            StartFullValidation();
            LastValidationTime = 0.0f;
        }
    }
}

void ABuildValidationSystem::StartFullValidation()
{
    if (bValidationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildValidationSystem: Validation already in progress"));
        return;
    }

    bValidationInProgress = true;
    OverallStatus = EBuild_ValidationStatus::Validating;
    TotalErrors = 0;
    TotalWarnings = 0;

    UE_LOG(LogTemp, Warning, TEXT("BuildValidationSystem: Starting full validation"));

    // Validate all modules
    for (FBuild_ModuleStatus& ModuleStatus : ModuleStatuses)
    {
        ModuleStatus = ValidateModuleInternal(ModuleStatus.ModuleName);
        TotalErrors += ModuleStatus.ErrorCount;
    }

    // Run integration tests
    RunIntegrationTests();

    // Check compilation and binary status
    CheckCompilationStatus();
    ValidateBinaryFiles();
    TestClassLoading();

    // Update overall status
    UpdateValidationStatus();

    bValidationInProgress = false;

    UE_LOG(LogTemp, Warning, TEXT("BuildValidationSystem: Full validation complete - Status: %d, Errors: %d"), 
           (int32)OverallStatus, TotalErrors);
}

void ABuildValidationSystem::ValidateModule(const FString& ModuleName)
{
    UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Validating module %s"), *ModuleName);

    // Find and update module status
    for (FBuild_ModuleStatus& ModuleStatus : ModuleStatuses)
    {
        if (ModuleStatus.ModuleName == ModuleName)
        {
            ModuleStatus = ValidateModuleInternal(ModuleName);
            break;
        }
    }
}

void ABuildValidationSystem::RunIntegrationTests()
{
    UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Running integration tests"));

    IntegrationTests.Empty();

    // Audio-VFX Integration Test
    TArray<FString> AudioVFXModules = {TEXT("Audio"), TEXT("VFX")};
    IntegrationTests.Add(RunIntegrationTestInternal(TEXT("Audio-VFX Integration"), AudioVFXModules));

    // World-Character Integration Test
    TArray<FString> WorldCharModules = {TEXT("WorldGeneration"), TEXT("Characters")};
    IntegrationTests.Add(RunIntegrationTestInternal(TEXT("World-Character Integration"), WorldCharModules));

    // AI-Combat Integration Test
    TArray<FString> AICombatModules = {TEXT("AI"), TEXT("Combat")};
    IntegrationTests.Add(RunIntegrationTestInternal(TEXT("AI-Combat Integration"), AICombatModules));

    // Core Systems Integration Test
    TArray<FString> CoreModules = {TEXT("Core"), TEXT("Physics"), TEXT("Performance")};
    IntegrationTests.Add(RunIntegrationTestInternal(TEXT("Core Systems Integration"), CoreModules));
}

EBuild_ValidationStatus ABuildValidationSystem::GetOverallStatus() const
{
    return OverallStatus;
}

TArray<FBuild_ModuleStatus> ABuildValidationSystem::GetModuleStatuses() const
{
    return ModuleStatuses;
}

TArray<FBuild_IntegrationTest> ABuildValidationSystem::GetIntegrationResults() const
{
    return IntegrationTests;
}

void ABuildValidationSystem::CheckCompilationStatus()
{
    UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Checking compilation status"));

    // Check if project compiles by looking for binary files
    FString ProjectDir = FPaths::ProjectDir();
    FString BinariesDir = FPaths::Combine(ProjectDir, TEXT("Binaries"));
    
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    
    if (PlatformFile.DirectoryExists(*BinariesDir))
    {
        UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Binaries directory found"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("BuildValidationSystem: Binaries directory not found - compilation may have failed"));
        TotalErrors++;
    }
}

void ABuildValidationSystem::ValidateBinaryFiles()
{
    UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Validating binary files"));

    FString ProjectDir = FPaths::ProjectDir();
    FString BinariesDir = FPaths::Combine(ProjectDir, TEXT("Binaries"), TEXT("Linux"));
    
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    
    TArray<FString> FoundFiles;
    PlatformFile.FindFiles(FoundFiles, *BinariesDir, TEXT(".so"));
    
    UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Found %d binary files"), FoundFiles.Num());
    
    if (FoundFiles.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildValidationSystem: No binary files found"));
        TotalWarnings++;
    }
}

void ABuildValidationSystem::TestClassLoading()
{
    UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Testing class loading"));

    // Test core classes
    TArray<FString> TestClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameMode"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.BuildIntegrationManager"),
        TEXT("/Script/TranspersonalGame.QATestManager")
    };

    int32 LoadedClasses = 0;
    for (const FString& ClassName : TestClasses)
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
        if (LoadedClass)
        {
            LoadedClasses++;
            UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Successfully loaded class %s"), *ClassName);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("BuildValidationSystem: Failed to load class %s"), *ClassName);
            TotalWarnings++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Loaded %d/%d test classes"), LoadedClasses, TestClasses.Num());
}

void ABuildValidationSystem::ValidateAudioVFXIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Validating Audio-VFX integration"));
    
    // Test if Audio and VFX managers can be loaded together
    UClass* AudioClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.AudioManager"));
    UClass* VFXClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.VFXManager"));
    
    if (AudioClass && VFXClass)
    {
        UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Audio-VFX integration test PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildValidationSystem: Audio-VFX integration test FAILED"));
        TotalWarnings++;
    }
}

void ABuildValidationSystem::ValidateWorldCharacterIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Validating World-Character integration"));
    
    // Test if World and Character systems can work together
    UClass* WorldGenClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
    UClass* CharacterClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    
    if (WorldGenClass && CharacterClass)
    {
        UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: World-Character integration test PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildValidationSystem: World-Character integration test FAILED"));
        TotalWarnings++;
    }
}

void ABuildValidationSystem::ValidateAICombatIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Validating AI-Combat integration"));
    
    // Test if AI and Combat systems can work together
    UClass* AIClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.NPCBehaviorManager"));
    UClass* CombatClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.CombatManager"));
    
    if (AIClass && CombatClass)
    {
        UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: AI-Combat integration test PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildValidationSystem: AI-Combat integration test FAILED"));
        TotalWarnings++;
    }
}

void ABuildValidationSystem::InitializeModuleList()
{
    ModuleStatuses.Empty();

    // Add all known modules
    TArray<FString> ModuleNames = {
        TEXT("Core"),
        TEXT("Physics"),
        TEXT("Performance"),
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

    for (const FString& ModuleName : ModuleNames)
    {
        FBuild_ModuleStatus ModuleStatus;
        ModuleStatus.ModuleName = ModuleName;
        ModuleStatus.Status = EBuild_ValidationStatus::Unknown;
        ModuleStatuses.Add(ModuleStatus);
    }
}

void ABuildValidationSystem::UpdateValidationStatus()
{
    // Determine overall status based on module statuses and errors
    if (TotalErrors > 10)
    {
        OverallStatus = EBuild_ValidationStatus::Critical;
    }
    else if (TotalErrors > 0)
    {
        OverallStatus = EBuild_ValidationStatus::Failed;
    }
    else if (TotalWarnings > 5)
    {
        OverallStatus = EBuild_ValidationStatus::Failed;
    }
    else
    {
        OverallStatus = EBuild_ValidationStatus::Passed;
    }

    UE_LOG(LogTemp, Warning, TEXT("BuildValidationSystem: Overall status updated to %d"), (int32)OverallStatus);
}

FBuild_ModuleStatus ABuildValidationSystem::ValidateModuleInternal(const FString& ModuleName)
{
    FBuild_ModuleStatus Status;
    Status.ModuleName = ModuleName;
    Status.Status = EBuild_ValidationStatus::Validating;
    Status.ErrorCount = 0;
    Status.LastError = TEXT("");
    Status.ValidationTime = GetWorld()->GetTimeSeconds();

    // Simple validation - check if module directory exists
    FString ModuleDir = FPaths::Combine(FPaths::ProjectDir(), TEXT("Source"), TEXT("TranspersonalGame"), ModuleName);
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

    if (PlatformFile.DirectoryExists(*ModuleDir))
    {
        Status.Status = EBuild_ValidationStatus::Passed;
        UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Module %s validation PASSED"), *ModuleName);
    }
    else
    {
        Status.Status = EBuild_ValidationStatus::Failed;
        Status.ErrorCount = 1;
        Status.LastError = FString::Printf(TEXT("Module directory not found: %s"), *ModuleDir);
        UE_LOG(LogTemp, Warning, TEXT("BuildValidationSystem: Module %s validation FAILED - %s"), *ModuleName, *Status.LastError);
    }

    return Status;
}

FBuild_IntegrationTest ABuildValidationSystem::RunIntegrationTestInternal(const FString& TestName, const TArray<FString>& RequiredModules)
{
    FBuild_IntegrationTest Test;
    Test.TestName = TestName;
    Test.RequiredModules = RequiredModules;
    Test.bTestPassed = true;
    Test.TestResult = TEXT("Test passed - all required modules available");

    // Check if all required modules exist
    for (const FString& ModuleName : RequiredModules)
    {
        FString ModuleDir = FPaths::Combine(FPaths::ProjectDir(), TEXT("Source"), TEXT("TranspersonalGame"), ModuleName);
        IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

        if (!PlatformFile.DirectoryExists(*ModuleDir))
        {
            Test.bTestPassed = false;
            Test.TestResult = FString::Printf(TEXT("Test failed - missing module: %s"), *ModuleName);
            break;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Integration test '%s' %s"), 
           *TestName, Test.bTestPassed ? TEXT("PASSED") : TEXT("FAILED"));

    return Test;
}
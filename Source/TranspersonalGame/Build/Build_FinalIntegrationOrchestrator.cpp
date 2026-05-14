#include "Build_FinalIntegrationOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

UBuild_FinalIntegrationOrchestrator::UBuild_FinalIntegrationOrchestrator()
{
    bIntegrationInProgress = false;
    IntegrationStartTime = 0.0f;
    
    // Initialize metrics
    IntegrationMetrics = FBuild_IntegrationMetrics();
    IntegrationMetrics.BuildStatus = EBuild_IntegrationStatus::Initializing;
}

void UBuild_FinalIntegrationOrchestrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Initializing final integration system"));
    
    InitializeCriticalSystems();
    IntegrationMetrics.BuildStatus = EBuild_IntegrationStatus::Validating;
    
    // Start automatic validation
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            ValidationTimerHandle,
            this,
            &UBuild_FinalIntegrationOrchestrator::ValidateAllSystems,
            5.0f,
            true
        );
    }
}

void UBuild_FinalIntegrationOrchestrator::Deinitialize()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(IntegrationTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(ValidationTimerHandle);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Deinitializing integration system"));
    Super::Deinitialize();
}

void UBuild_FinalIntegrationOrchestrator::InitializeCriticalSystems()
{
    CriticalSystemPaths.Empty();
    CriticalSystemPaths.Add(TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    CriticalSystemPaths.Add(TEXT("/Script/TranspersonalGame.TranspersonalGameMode"));
    CriticalSystemPaths.Add(TEXT("/Script/TranspersonalGame.VFX_ImpactManager"));
    CriticalSystemPaths.Add(TEXT("/Script/TranspersonalGame.QA_SystemValidator"));
    
    IntegrationMetrics.TotalSystems = CriticalSystemPaths.Num();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Initialized %d critical systems"), IntegrationMetrics.TotalSystems);
}

void UBuild_FinalIntegrationOrchestrator::StartFinalIntegration()
{
    if (bIntegrationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Integration already in progress"));
        return;
    }
    
    bIntegrationInProgress = true;
    IntegrationStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    IntegrationMetrics.BuildStatus = EBuild_IntegrationStatus::Integrating;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Starting final integration process"));
    
    // Clear previous results
    ValidationResults.Empty();
    
    // Start integration timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            IntegrationTimerHandle,
            this,
            &UBuild_FinalIntegrationOrchestrator::OnIntegrationComplete,
            30.0f,
            false
        );
    }
    
    // Begin validation
    ValidateAllSystems();
}

void UBuild_FinalIntegrationOrchestrator::ValidateAllSystems()
{
    if (!bIntegrationInProgress && IntegrationMetrics.BuildStatus != EBuild_IntegrationStatus::Validating)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Validating all systems"));
    
    ValidationResults.Empty();
    IntegrationMetrics.LoadedSystems = 0;
    IntegrationMetrics.OperationalSystems = 0;
    
    // Validate each critical system
    for (const FString& SystemPath : CriticalSystemPaths)
    {
        FString SystemName = SystemPath;
        if (SystemPath.Contains(TEXT(".")))
        {
            SystemPath.Split(TEXT("."), nullptr, &SystemName);
        }
        
        FBuild_SystemValidationResult Result = ValidateSystemInternal(SystemName, SystemPath);
        ValidationResults.Add(Result);
        
        if (Result.bIsLoaded)
        {
            IntegrationMetrics.LoadedSystems++;
        }
        
        if (Result.bIsOperational)
        {
            IntegrationMetrics.OperationalSystems++;
        }
    }
    
    // Validate world state
    ValidateWorldState();
    ValidateCharacterSystems();
    ValidateVFXSystems();
    ValidateQASystems();
    
    UpdateIntegrationMetrics();
    LogIntegrationStatus();
}

FBuild_SystemValidationResult UBuild_FinalIntegrationOrchestrator::ValidateSystem(const FString& SystemName, const FString& ClassPath)
{
    return ValidateSystemInternal(SystemName, ClassPath);
}

FBuild_SystemValidationResult UBuild_FinalIntegrationOrchestrator::ValidateSystemInternal(const FString& SystemName, const FString& ClassPath)
{
    FBuild_SystemValidationResult Result;
    Result.SystemName = SystemName;
    Result.ValidationTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Try to load the class
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
    if (LoadedClass)
    {
        Result.bIsLoaded = true;
        Result.bIsOperational = true; // Assume operational if loaded
        Result.ValidationMessage = FString::Printf(TEXT("%s loaded successfully"), *SystemName);
        
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: ✅ %s - LOADED"), *SystemName);
    }
    else
    {
        Result.bIsLoaded = false;
        Result.bIsOperational = false;
        Result.ValidationMessage = FString::Printf(TEXT("%s failed to load"), *SystemName);
        
        UE_LOG(LogTemp, Error, TEXT("Build_FinalIntegrationOrchestrator: ❌ %s - FAILED"), *SystemName);
    }
    
    return Result;
}

bool UBuild_FinalIntegrationOrchestrator::ValidateWorldState()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        IntegrationMetrics.WorldActorCount = 0;
        return false;
    }
    
    // Count all actors in the world
    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ActorCount++;
    }
    
    IntegrationMetrics.WorldActorCount = ActorCount;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: World validation - %d actors"), ActorCount);
    return ActorCount > 0;
}

bool UBuild_FinalIntegrationOrchestrator::ValidateCharacterSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        IntegrationMetrics.CharacterInstances = 0;
        return false;
    }
    
    // Count character instances
    UClass* CharacterClass = LoadClass<APawn>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    if (CharacterClass)
    {
        TArray<AActor*> CharacterActors;
        UGameplayStatics::GetAllActorsOfClass(World, CharacterClass, CharacterActors);
        IntegrationMetrics.CharacterInstances = CharacterActors.Num();
        
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Character validation - %d instances"), CharacterActors.Num());
        return true;
    }
    
    IntegrationMetrics.CharacterInstances = 0;
    return false;
}

bool UBuild_FinalIntegrationOrchestrator::ValidateVFXSystems()
{
    UClass* VFXClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.VFX_ImpactManager"));
    if (VFXClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: VFX validation - PASS"));
        return true;
    }
    
    UE_LOG(LogTemp, Error, TEXT("Build_FinalIntegrationOrchestrator: VFX validation - FAIL"));
    return false;
}

bool UBuild_FinalIntegrationOrchestrator::ValidateQASystems()
{
    UClass* QAClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.QA_SystemValidator"));
    if (QAClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: QA validation - PASS"));
        return true;
    }
    
    UE_LOG(LogTemp, Error, TEXT("Build_FinalIntegrationOrchestrator: QA validation - FAIL"));
    return false;
}

void UBuild_FinalIntegrationOrchestrator::UpdateIntegrationMetrics()
{
    // Calculate total validation time
    if (GetWorld())
    {
        IntegrationMetrics.TotalValidationTime = GetWorld()->GetTimeSeconds() - IntegrationStartTime;
    }
    
    // Determine build status
    if (IntegrationMetrics.LoadedSystems >= 3 && IntegrationMetrics.OperationalSystems >= 3)
    {
        if (IntegrationMetrics.WorldActorCount > 0)
        {
            IntegrationMetrics.BuildStatus = EBuild_IntegrationStatus::Stable;
            
            if (IntegrationMetrics.LoadedSystems == IntegrationMetrics.TotalSystems)
            {
                IntegrationMetrics.BuildStatus = EBuild_IntegrationStatus::Ready;
            }
        }
        else
        {
            IntegrationMetrics.BuildStatus = EBuild_IntegrationStatus::Testing;
        }
    }
    else
    {
        IntegrationMetrics.BuildStatus = EBuild_IntegrationStatus::Failed;
    }
}

void UBuild_FinalIntegrationOrchestrator::LogIntegrationStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: === INTEGRATION STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("  Systems Loaded: %d/%d"), IntegrationMetrics.LoadedSystems, IntegrationMetrics.TotalSystems);
    UE_LOG(LogTemp, Warning, TEXT("  Systems Operational: %d/%d"), IntegrationMetrics.OperationalSystems, IntegrationMetrics.TotalSystems);
    UE_LOG(LogTemp, Warning, TEXT("  World Actors: %d"), IntegrationMetrics.WorldActorCount);
    UE_LOG(LogTemp, Warning, TEXT("  Character Instances: %d"), IntegrationMetrics.CharacterInstances);
    UE_LOG(LogTemp, Warning, TEXT("  Build Status: %s"), *UEnum::GetValueAsString(IntegrationMetrics.BuildStatus));
    UE_LOG(LogTemp, Warning, TEXT("  Validation Time: %.2fs"), IntegrationMetrics.TotalValidationTime);
}

void UBuild_FinalIntegrationOrchestrator::RunIntegrationTests()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Running integration tests"));
    
    IntegrationMetrics.BuildStatus = EBuild_IntegrationStatus::Testing;
    
    // Run comprehensive tests
    bool bAllTestsPassed = true;
    
    bAllTestsPassed &= ValidateWorldState();
    bAllTestsPassed &= ValidateCharacterSystems();
    bAllTestsPassed &= ValidateVFXSystems();
    bAllTestsPassed &= ValidateQASystems();
    
    if (bAllTestsPassed)
    {
        IntegrationMetrics.BuildStatus = EBuild_IntegrationStatus::Ready;
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: All integration tests PASSED"));
    }
    else
    {
        IntegrationMetrics.BuildStatus = EBuild_IntegrationStatus::Failed;
        UE_LOG(LogTemp, Error, TEXT("Build_FinalIntegrationOrchestrator: Integration tests FAILED"));
    }
}

void UBuild_FinalIntegrationOrchestrator::GenerateBuildReport()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: === FINAL BUILD REPORT ==="));
    
    LogIntegrationStatus();
    
    UE_LOG(LogTemp, Warning, TEXT("=== VALIDATION RESULTS ==="));
    for (const FBuild_SystemValidationResult& Result : ValidationResults)
    {
        FString Status = Result.bIsOperational ? TEXT("✅ PASS") : TEXT("❌ FAIL");
        UE_LOG(LogTemp, Warning, TEXT("  %s: %s - %s"), *Result.SystemName, *Status, *Result.ValidationMessage);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== BUILD READY: %s ==="), IsBuildReady() ? TEXT("YES") : TEXT("NO"));
}

void UBuild_FinalIntegrationOrchestrator::PrepareBuildPackaging()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Preparing build for packaging"));
    
    if (IntegrationMetrics.BuildStatus == EBuild_IntegrationStatus::Ready)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Build is ready for packaging"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Build_FinalIntegrationOrchestrator: Build is NOT ready for packaging - Status: %s"), 
               *UEnum::GetValueAsString(IntegrationMetrics.BuildStatus));
    }
}

void UBuild_FinalIntegrationOrchestrator::FinalizeIntegration()
{
    bIntegrationInProgress = false;
    
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(IntegrationTimerHandle);
    }
    
    GenerateBuildReport();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Integration finalized"));
}

void UBuild_FinalIntegrationOrchestrator::OnIntegrationComplete()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Integration timer completed"));
    
    RunIntegrationTests();
    FinalizeIntegration();
}

void UBuild_FinalIntegrationOrchestrator::OnValidationComplete()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Validation cycle completed"));
    
    UpdateIntegrationMetrics();
}
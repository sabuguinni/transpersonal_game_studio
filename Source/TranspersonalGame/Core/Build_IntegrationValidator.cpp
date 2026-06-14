#include "Build_IntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"

UBuild_IntegrationValidator::UBuild_IntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f;
    
    MaxActorCount = 8000;
    MaxDinosaurCount = 150;
    ValidationInterval = 30.0f;
    TimeSinceLastValidation = 0.0f;
    
    // Initialize core systems to validate
    CoreSystemsToValidate.Add(TEXT("TranspersonalCharacter"));
    CoreSystemsToValidate.Add(TEXT("TranspersonalGameState"));
    CoreSystemsToValidate.Add(TEXT("PCGWorldGenerator"));
    CoreSystemsToValidate.Add(TEXT("FoliageManager"));
    CoreSystemsToValidate.Add(TEXT("CrowdSimulationManager"));
    CoreSystemsToValidate.Add(TEXT("ProceduralWorldManager"));
}

void UBuild_IntegrationValidator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationValidator: Component initialized"));
    
    // Perform initial validation
    ValidateAllSystems();
}

void UBuild_IntegrationValidator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    TimeSinceLastValidation += DeltaTime;
    
    if (TimeSinceLastValidation >= ValidationInterval)
    {
        ValidateAllSystems();
        EnforceActorCaps();
        TimeSinceLastValidation = 0.0f;
    }
}

FBuild_IntegrationReport UBuild_IntegrationValidator::GenerateIntegrationReport()
{
    FBuild_IntegrationReport Report;
    
    // Get total actor count
    Report.TotalActors = GetTotalActorCount();
    Report.TotalSystems = CoreSystemsToValidate.Num();
    
    // Validate each system
    int32 WorkingSystemCount = 0;
    for (const FString& SystemName : CoreSystemsToValidate)
    {
        FBuild_SystemHealth SystemHealth = ValidateIndividualSystem(SystemName);
        Report.SystemHealthData.Add(SystemHealth);
        
        if (SystemHealth.bIsOperational)
        {
            WorkingSystemCount++;
        }
    }
    
    Report.WorkingSystems = WorkingSystemCount;
    Report.IntegrationScore = (float)WorkingSystemCount / (float)Report.TotalSystems * 100.0f;
    
    // Determine performance status
    if (Report.TotalActors < 1000)
    {
        Report.PerformanceStatus = TEXT("OPTIMAL");
    }
    else if (Report.TotalActors < 5000)
    {
        Report.PerformanceStatus = TEXT("GOOD");
    }
    else if (Report.TotalActors < 8000)
    {
        Report.PerformanceStatus = TEXT("ACCEPTABLE");
    }
    else
    {
        Report.PerformanceStatus = TEXT("CRITICAL");
    }
    
    // Calculate overall status
    Report.OverallStatus = CalculateOverallStatus();
    
    // Generate recommendations
    if (Report.IntegrationScore < 75.0f)
    {
        Report.Recommendations = TEXT("FOCUS_ON_CORE_SYSTEMS");
    }
    else if (Report.TotalActors > 7000)
    {
        Report.Recommendations = TEXT("OPTIMIZE_ACTOR_COUNT");
    }
    else
    {
        Report.Recommendations = TEXT("EXPAND_FEATURES");
    }
    
    LastReport = Report;
    LogIntegrationStatus(Report);
    
    return Report;
}

bool UBuild_IntegrationValidator::ValidateSystemHealth(const FString& SystemName)
{
    FBuild_SystemHealth SystemHealth = ValidateIndividualSystem(SystemName);
    return SystemHealth.bIsOperational;
}

void UBuild_IntegrationValidator::EnforceActorCaps()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 TotalActors = AllActors.Num();
    
    if (TotalActors > MaxActorCount)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationValidator: Actor count (%d) exceeds maximum (%d), enforcing caps"), TotalActors, MaxActorCount);
        CleanupExcessActors();
    }
}

int32 UBuild_IntegrationValidator::GetTotalActorCount() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    return AllActors.Num();
}

TArray<FString> UBuild_IntegrationValidator::GetLoadedModules() const
{
    TArray<FString> LoadedModules;
    
    for (const FString& SystemName : CoreSystemsToValidate)
    {
        if (ValidateSystemHealth(SystemName))
        {
            LoadedModules.Add(SystemName);
        }
    }
    
    return LoadedModules;
}

EBuild_IntegrationStatus UBuild_IntegrationValidator::CalculateOverallStatus() const
{
    if (LastReport.IntegrationScore >= 90.0f && LastReport.TotalActors < 5000)
    {
        return EBuild_IntegrationStatus::Stable;
    }
    else if (LastReport.IntegrationScore >= 75.0f && LastReport.TotalActors < 8000)
    {
        return EBuild_IntegrationStatus::Functional;
    }
    else if (LastReport.IntegrationScore >= 50.0f)
    {
        return EBuild_IntegrationStatus::Unstable;
    }
    else
    {
        return EBuild_IntegrationStatus::Critical;
    }
}

void UBuild_IntegrationValidator::ValidateAllSystems()
{
    GenerateIntegrationReport();
}

FBuild_SystemHealth UBuild_IntegrationValidator::ValidateIndividualSystem(const FString& SystemName)
{
    FBuild_SystemHealth SystemHealth;
    SystemHealth.SystemName = SystemName;
    
    // Try to load the class
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *SystemName);
    UClass* SystemClass = LoadClass<UObject>(nullptr, *ClassPath);
    
    if (SystemClass)
    {
        SystemHealth.bIsOperational = true;
        SystemHealth.HealthScore = 100.0f;
        SystemHealth.LastError = TEXT("");
        
        UE_LOG(LogTemp, Log, TEXT("Build_IntegrationValidator: System %s is operational"), *SystemName);
    }
    else
    {
        SystemHealth.bIsOperational = false;
        SystemHealth.HealthScore = 0.0f;
        SystemHealth.LastError = TEXT("Failed to load class");
        
        UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationValidator: System %s failed to load"), *SystemName);
    }
    
    return SystemHealth;
}

void UBuild_IntegrationValidator::CleanupExcessActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    // Remove excess actors (keep essential ones)
    int32 ActorsToRemove = AllActors.Num() - MaxActorCount;
    int32 RemovedCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (RemovedCount >= ActorsToRemove)
        {
            break;
        }
        
        if (Actor && !Actor->IsA<APawn>() && !Actor->GetName().Contains(TEXT("Light")) && !Actor->GetName().Contains(TEXT("PlayerStart")))
        {
            Actor->Destroy();
            RemovedCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationValidator: Removed %d excess actors"), RemovedCount);
}

void UBuild_IntegrationValidator::LogIntegrationStatus(const FBuild_IntegrationReport& Report)
{
    UE_LOG(LogTemp, Warning, TEXT("=== INTEGRATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Overall Status: %s"), *UEnum::GetValueAsString(Report.OverallStatus));
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), Report.TotalActors);
    UE_LOG(LogTemp, Warning, TEXT("Working Systems: %d/%d"), Report.WorkingSystems, Report.TotalSystems);
    UE_LOG(LogTemp, Warning, TEXT("Integration Score: %.1f%%"), Report.IntegrationScore);
    UE_LOG(LogTemp, Warning, TEXT("Performance Status: %s"), *Report.PerformanceStatus);
    UE_LOG(LogTemp, Warning, TEXT("Recommendations: %s"), *Report.Recommendations);
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}
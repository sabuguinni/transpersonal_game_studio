#include "QA_ValidationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "UObject/UObjectGlobals.h"

UQA_ValidationManager::UQA_ValidationManager()
{
    TotalActorCount = 0;
    DinosaurCount = 0;
    LastValidationTime = 0.0f;
}

void UQA_ValidationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("QA_ValidationManager initialized"));
}

void UQA_ValidationManager::Deinitialize()
{
    ValidationReports.Empty();
    Super::Deinitialize();
}

void UQA_ValidationManager::RunFullValidationSuite()
{
    UE_LOG(LogTemp, Log, TEXT("QA: Starting full validation suite"));
    
    double StartTime = FPlatformTime::Seconds();
    ClearValidationReports();

    // Run all validation tests
    ValidateActorDistribution();
    ValidatePerformanceMetrics();
    ValidateGameplaySystems();
    ValidateDinosaurAssets();
    ValidateVFXSystems();

    LastValidationTime = FPlatformTime::Seconds() - StartTime;
    
    UE_LOG(LogTemp, Log, TEXT("QA: Full validation suite completed in %.2f seconds"), LastValidationTime);
    
    // Log summary
    int32 PassCount = 0, WarningCount = 0, FailCount = 0, CriticalCount = 0;
    for (const FQA_ValidationReport& Report : ValidationReports)
    {
        switch (Report.Result)
        {
            case EQA_ValidationResult::Pass: PassCount++; break;
            case EQA_ValidationResult::Warning: WarningCount++; break;
            case EQA_ValidationResult::Fail: FailCount++; break;
            case EQA_ValidationResult::Critical: CriticalCount++; break;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("QA Summary: %d Pass, %d Warning, %d Fail, %d Critical"), 
           PassCount, WarningCount, FailCount, CriticalCount);
}

void UQA_ValidationManager::ValidateActorDistribution()
{
    double StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationReport(TEXT("Actor Distribution"), EQA_ValidationResult::Critical, 
                          TEXT("World not found"), FPlatformTime::Seconds() - StartTime);
        return;
    }

    TArray<AActor*> AllActors;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AllActors.Add(*ActorItr);
    }

    TotalActorCount = AllActors.Num();
    
    // Validate biome distribution
    ValidateBiomeDistribution();
    
    // Performance check
    EQA_ValidationResult Result = EQA_ValidationResult::Pass;
    FString Details = FString::Printf(TEXT("Total actors: %d"), TotalActorCount);
    
    if (TotalActorCount > 50000)
    {
        Result = EQA_ValidationResult::Critical;
        Details += TEXT(" - CRITICAL: Too many actors, severe performance impact expected");
    }
    else if (TotalActorCount > 20000)
    {
        Result = EQA_ValidationResult::Warning;
        Details += TEXT(" - WARNING: High actor count, monitor performance");
    }
    else
    {
        Details += TEXT(" - GOOD: Actor count within acceptable range");
    }

    AddValidationReport(TEXT("Actor Distribution"), Result, Details, FPlatformTime::Seconds() - StartTime);
}

void UQA_ValidationManager::ValidatePerformanceMetrics()
{
    double StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationReport(TEXT("Performance Metrics"), EQA_ValidationResult::Critical, 
                          TEXT("World not found"), FPlatformTime::Seconds() - StartTime);
        return;
    }

    int32 StaticMeshCount = 0;
    int32 SkeletalMeshCount = 0;
    int32 LightCount = 0;
    int32 CustomClassCount = 0;

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;

        FString ClassName = Actor->GetClass()->GetName();
        
        if (ClassName.Contains(TEXT("StaticMesh")))
        {
            StaticMeshCount++;
        }
        else if (ClassName.Contains(TEXT("SkeletalMesh")) || ClassName.Contains(TEXT("Pawn")))
        {
            SkeletalMeshCount++;
        }
        else if (ClassName.Contains(TEXT("Light")))
        {
            LightCount++;
        }
        else if (ClassName.Contains(TEXT("Transpersonal")) || ClassName.Contains(TEXT("Dinosaur")))
        {
            CustomClassCount++;
        }
    }

    FString Details = FString::Printf(TEXT("Static: %d, Skeletal: %d, Lights: %d, Custom: %d"), 
                                    StaticMeshCount, SkeletalMeshCount, LightCount, CustomClassCount);

    EQA_ValidationResult Result = EQA_ValidationResult::Pass;
    if (StaticMeshCount > 30000 || SkeletalMeshCount > 1000)
    {
        Result = EQA_ValidationResult::Warning;
        Details += TEXT(" - High mesh count detected");
    }

    AddValidationReport(TEXT("Performance Metrics"), Result, Details, FPlatformTime::Seconds() - StartTime);
}

void UQA_ValidationManager::ValidateGameplaySystems()
{
    double StartTime = FPlatformTime::Seconds();
    
    // Test core class loading
    TArray<FString> ClassesToTest = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameMode"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState")
    };

    int32 LoadedClasses = 0;
    FString FailedClasses;

    for (const FString& ClassName : ClassesToTest)
    {
        UClass* TestClass = LoadClass<UObject>(nullptr, *ClassName);
        if (TestClass)
        {
            LoadedClasses++;
        }
        else
        {
            if (!FailedClasses.IsEmpty()) FailedClasses += TEXT(", ");
            FailedClasses += ClassName;
        }
    }

    EQA_ValidationResult Result = (LoadedClasses == ClassesToTest.Num()) ? 
                                 EQA_ValidationResult::Pass : EQA_ValidationResult::Fail;
    
    FString Details = FString::Printf(TEXT("Loaded %d/%d core classes"), LoadedClasses, ClassesToTest.Num());
    if (!FailedClasses.IsEmpty())
    {
        Details += FString::Printf(TEXT(" - Failed: %s"), *FailedClasses);
    }

    AddValidationReport(TEXT("Gameplay Systems"), Result, Details, FPlatformTime::Seconds() - StartTime);
}

void UQA_ValidationManager::ValidateDinosaurAssets()
{
    double StartTime = FPlatformTime::Seconds();
    
    TArray<FString> DinosaurAssets = {
        TEXT("/Game/Dinosaur_Pack/Trex/Mesh/SKM_Trex_Skin"),
        TEXT("/Game/Dinosaur_Pack/Velociraptor/Mesh/SKM_Velociraptor_Skin"),
        TEXT("/Game/Dinosaur_Pack/Triceratops/Mesh/SKM_Triceratops")
    };

    int32 LoadedAssets = 0;
    FString FailedAssets;

    for (const FString& AssetPath : DinosaurAssets)
    {
        UObject* Asset = LoadObject<UObject>(nullptr, *AssetPath);
        if (Asset)
        {
            LoadedAssets++;
        }
        else
        {
            if (!FailedAssets.IsEmpty()) FailedAssets += TEXT(", ");
            FailedAssets += AssetPath;
        }
    }

    // Count dinosaur actors in world
    DinosaurCount = 0;
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetClass()->GetName().Contains(TEXT("Dinosaur")))
            {
                DinosaurCount++;
            }
        }
    }

    EQA_ValidationResult Result = (LoadedAssets == DinosaurAssets.Num()) ? 
                                 EQA_ValidationResult::Pass : EQA_ValidationResult::Warning;
    
    FString Details = FString::Printf(TEXT("Assets: %d/%d loaded, Spawned: %d dinosaurs"), 
                                    LoadedAssets, DinosaurAssets.Num(), DinosaurCount);
    if (!FailedAssets.IsEmpty())
    {
        Details += FString::Printf(TEXT(" - Failed: %s"), *FailedAssets);
    }

    AddValidationReport(TEXT("Dinosaur Assets"), Result, Details, FPlatformTime::Seconds() - StartTime);
}

void UQA_ValidationManager::ValidateVFXSystems()
{
    double StartTime = FPlatformTime::Seconds();
    
    // Test VFX class loading
    UClass* VFXClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.VFX_ImpactManager"));
    
    EQA_ValidationResult Result = VFXClass ? EQA_ValidationResult::Pass : EQA_ValidationResult::Warning;
    FString Details = VFXClass ? TEXT("VFX_ImpactManager loaded successfully") : 
                                TEXT("VFX_ImpactManager not found");

    AddValidationReport(TEXT("VFX Systems"), Result, Details, FPlatformTime::Seconds() - StartTime);
}

TArray<FQA_ValidationReport> UQA_ValidationManager::GetValidationReports() const
{
    return ValidationReports;
}

void UQA_ValidationManager::ClearValidationReports()
{
    ValidationReports.Empty();
}

void UQA_ValidationManager::ExportValidationReport(const FString& FilePath)
{
    FString ReportContent = FString::Printf(TEXT("QA Validation Report - %s\n"), 
                                          *FDateTime::Now().ToString());
    ReportContent += FString::Printf(TEXT("Total Validation Time: %.2f seconds\n\n"), LastValidationTime);

    for (const FQA_ValidationReport& Report : ValidationReports)
    {
        FString ResultString;
        switch (Report.Result)
        {
            case EQA_ValidationResult::Pass: ResultString = TEXT("PASS"); break;
            case EQA_ValidationResult::Warning: ResultString = TEXT("WARNING"); break;
            case EQA_ValidationResult::Fail: ResultString = TEXT("FAIL"); break;
            case EQA_ValidationResult::Critical: ResultString = TEXT("CRITICAL"); break;
        }

        ReportContent += FString::Printf(TEXT("[%s] %s (%.3fs)\n%s\n\n"), 
                                       *ResultString, *Report.TestName, 
                                       Report.ExecutionTime, *Report.Details);
    }

    FFileHelper::SaveStringToFile(ReportContent, *FilePath);
    UE_LOG(LogTemp, Log, TEXT("QA: Validation report exported to %s"), *FilePath);
}

void UQA_ValidationManager::AddValidationReport(const FString& TestName, EQA_ValidationResult Result, 
                                              const FString& Details, float ExecutionTime)
{
    FQA_ValidationReport Report;
    Report.TestName = TestName;
    Report.Result = Result;
    Report.Details = Details;
    Report.ExecutionTime = ExecutionTime;
    
    ValidationReports.Add(Report);
    
    UE_LOG(LogTemp, Log, TEXT("QA: %s - %s"), *TestName, *Details);
}

void UQA_ValidationManager::ValidateBiomeDistribution()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Define biome centers
    TArray<TPair<FString, FVector>> Biomes = {
        {TEXT("Savana"), FVector(0, 0, 0)},
        {TEXT("Pantano"), FVector(-50000, -45000, 0)},
        {TEXT("Floresta"), FVector(-45000, 40000, 0)},
        {TEXT("Deserto"), FVector(55000, 0, 0)},
        {TEXT("Montanha"), FVector(40000, 50000, 0)}
    };

    TMap<FString, int32> BiomeCounts;
    for (const auto& Biome : Biomes)
    {
        BiomeCounts.Add(Biome.Key, 0);
    }
    BiomeCounts.Add(TEXT("Other"), 0);

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;

        FVector ActorLocation = Actor->GetActorLocation();
        FString ClosestBiome = TEXT("Other");
        float MinDistance = 20000.0f; // 20km radius

        for (const auto& Biome : Biomes)
        {
            float Distance = FVector::Dist2D(ActorLocation, Biome.Value);
            if (Distance < MinDistance)
            {
                MinDistance = Distance;
                ClosestBiome = Biome.Key;
            }
        }

        BiomeCounts[ClosestBiome]++;
    }

    // Log biome distribution
    for (const auto& Count : BiomeCounts)
    {
        UE_LOG(LogTemp, Log, TEXT("QA: %s biome has %d actors"), *Count.Key, Count.Value);
    }
}

bool UQA_ValidationManager::IsActorInBiome(AActor* Actor, const FVector& BiomeCenter, float Radius)
{
    if (!Actor) return false;
    
    FVector ActorLocation = Actor->GetActorLocation();
    float Distance = FVector::Dist2D(ActorLocation, BiomeCenter);
    return Distance <= Radius;
}
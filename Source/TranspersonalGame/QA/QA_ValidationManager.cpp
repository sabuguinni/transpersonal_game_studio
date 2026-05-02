#include "QA_ValidationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Landscape/Landscape.h"
#include "Engine/StaticMeshActor.h"
#include "UObject/UObjectGlobals.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"

UQA_ValidationManager::UQA_ValidationManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    bAutoRunOnBeginPlay = false;
    bLogDetailedResults = true;
    ValidationTimeoutSeconds = 30.0f;
    
    InitializeDefaultSettings();
}

void UQA_ValidationManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoRunOnBeginPlay)
    {
        FQA_ValidationSuite ValidationResults = RunFullValidationSuite();
        LogValidationResults(ValidationResults);
    }
}

void UQA_ValidationManager::InitializeDefaultSettings()
{
    // Classes críticas que devem estar sempre carregáveis
    CriticalClassPaths.Empty();
    CriticalClassPaths.Add(TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    CriticalClassPaths.Add(TEXT("/Script/TranspersonalGame.TranspersonalGameMode"));
    CriticalClassPaths.Add(TEXT("/Script/TranspersonalGame.VFX_ImpactManager"));
    
    // Tipos de actores essenciais e quantidades mínimas
    EssentialActorTypes.Empty();
    EssentialActorTypes.Add(TEXT("PlayerStart"), 1);
    EssentialActorTypes.Add(TEXT("DirectionalLight"), 1);
    EssentialActorTypes.Add(TEXT("SkyAtmosphere"), 1);
    EssentialActorTypes.Add(TEXT("Landscape"), 1);
}

FQA_ValidationSuite UQA_ValidationManager::RunFullValidationSuite()
{
    FQA_ValidationSuite ValidationSuite;
    ValidationSuite.ExecutionTime = FDateTime::Now();
    
    UE_LOG(LogTemp, Warning, TEXT("QA Validation Suite: Starting full validation..."));
    
    // Executar todos os testes
    ValidationSuite.TestResults.Add(ValidateEssentialActors());
    ValidationSuite.TestResults.Add(ValidateClassLoading());
    ValidationSuite.TestResults.Add(ValidateLightingSetup());
    ValidationSuite.TestResults.Add(ValidateVFXSystems());
    ValidationSuite.TestResults.Add(ValidateCharacterSystems());
    
    // Calcular estatísticas
    ValidationSuite.TotalTests = ValidationSuite.TestResults.Num();
    ValidationSuite.PassedTests = 0;
    
    for (const FQA_ValidationResult& Result : ValidationSuite.TestResults)
    {
        if (Result.bPassed)
        {
            ValidationSuite.PassedTests++;
        }
    }
    
    ValidationSuite.OverallScore = ValidationSuite.TotalTests > 0 ? 
        (float(ValidationSuite.PassedTests) / float(ValidationSuite.TotalTests)) * 100.0f : 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("QA Validation Suite: Completed with score %.1f%% (%d/%d tests passed)"), 
        ValidationSuite.OverallScore, ValidationSuite.PassedTests, ValidationSuite.TotalTests);
    
    return ValidationSuite;
}

FQA_ValidationResult UQA_ValidationManager::ValidateEssentialActors()
{
    float StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return CreateTestResult(TEXT("Essential Actors"), false, TEXT("No valid world found"), 0.0f);
    }
    
    TArray<FString> MissingActors;
    
    for (const auto& ActorTypePair : EssentialActorTypes)
    {
        FString ActorType = ActorTypePair.Key;
        int32 RequiredCount = ActorTypePair.Value;
        
        TArray<AActor*> FoundActors = GetActorsOfType(ActorType);
        
        if (FoundActors.Num() < RequiredCount)
        {
            MissingActors.Add(FString::Printf(TEXT("%s (found %d, required %d)"), 
                *ActorType, FoundActors.Num(), RequiredCount));
        }
    }
    
    float ExecutionTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
    
    if (MissingActors.Num() > 0)
    {
        FString ErrorMsg = TEXT("Missing essential actors: ") + FString::Join(MissingActors, TEXT(", "));
        return CreateTestResult(TEXT("Essential Actors"), false, ErrorMsg, ExecutionTime);
    }
    
    return CreateTestResult(TEXT("Essential Actors"), true, TEXT("All essential actors present"), ExecutionTime);
}

FQA_ValidationResult UQA_ValidationManager::ValidateClassLoading()
{
    float StartTime = FPlatformTime::Seconds();
    
    TArray<FString> FailedClasses;
    
    for (const FString& ClassPath : CriticalClassPaths)
    {
        if (!IsClassLoadable(ClassPath))
        {
            FailedClasses.Add(ClassPath);
        }
    }
    
    float ExecutionTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
    
    if (FailedClasses.Num() > 0)
    {
        FString ErrorMsg = TEXT("Failed to load classes: ") + FString::Join(FailedClasses, TEXT(", "));
        return CreateTestResult(TEXT("Class Loading"), false, ErrorMsg, ExecutionTime);
    }
    
    return CreateTestResult(TEXT("Class Loading"), true, TEXT("All critical classes loaded successfully"), ExecutionTime);
}

FQA_ValidationResult UQA_ValidationManager::ValidateLightingSetup()
{
    float StartTime = FPlatformTime::Seconds();
    
    TArray<FString> LightingIssues;
    
    // Verificar duplicados de lighting
    TArray<FString> LightingTypes = {TEXT("DirectionalLight"), TEXT("SkyAtmosphere"), TEXT("SkyLight"), TEXT("ExponentialHeightFog")};
    
    for (const FString& LightingType : LightingTypes)
    {
        TArray<AActor*> LightingActors = GetActorsOfType(LightingType);
        if (LightingActors.Num() > 1)
        {
            LightingIssues.Add(FString::Printf(TEXT("%s has %d instances (should be 1)"), 
                *LightingType, LightingActors.Num()));
        }
    }
    
    float ExecutionTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
    
    if (LightingIssues.Num() > 0)
    {
        FString ErrorMsg = TEXT("Lighting issues: ") + FString::Join(LightingIssues, TEXT(", "));
        return CreateTestResult(TEXT("Lighting Setup"), false, ErrorMsg, ExecutionTime);
    }
    
    return CreateTestResult(TEXT("Lighting Setup"), true, TEXT("Lighting setup is correct"), ExecutionTime);
}

FQA_ValidationResult UQA_ValidationManager::ValidateVFXSystems()
{
    float StartTime = FPlatformTime::Seconds();
    
    // Verificar se a classe VFX_ImpactManager está carregável
    bool bVFXManagerLoadable = IsClassLoadable(TEXT("/Script/TranspersonalGame.VFX_ImpactManager"));
    
    float ExecutionTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
    
    if (!bVFXManagerLoadable)
    {
        return CreateTestResult(TEXT("VFX Systems"), false, TEXT("VFX_ImpactManager class not loadable"), ExecutionTime);
    }
    
    return CreateTestResult(TEXT("VFX Systems"), true, TEXT("VFX systems functional"), ExecutionTime);
}

FQA_ValidationResult UQA_ValidationManager::ValidateCharacterSystems()
{
    float StartTime = FPlatformTime::Seconds();
    
    // Verificar se TranspersonalCharacter está carregável
    bool bCharacterLoadable = IsClassLoadable(TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    
    // Verificar se existe PlayerStart
    TArray<AActor*> PlayerStarts = GetActorsOfType(TEXT("PlayerStart"));
    
    float ExecutionTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
    
    if (!bCharacterLoadable)
    {
        return CreateTestResult(TEXT("Character Systems"), false, TEXT("TranspersonalCharacter class not loadable"), ExecutionTime);
    }
    
    if (PlayerStarts.Num() == 0)
    {
        return CreateTestResult(TEXT("Character Systems"), false, TEXT("No PlayerStart found in level"), ExecutionTime);
    }
    
    return CreateTestResult(TEXT("Character Systems"), true, TEXT("Character systems functional"), ExecutionTime);
}

void UQA_ValidationManager::LogValidationResults(const FQA_ValidationSuite& ValidationSuite)
{
    UE_LOG(LogTemp, Warning, TEXT("=== QA VALIDATION RESULTS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Overall Score: %.1f%% (%d/%d tests passed)"), 
        ValidationSuite.OverallScore, ValidationSuite.PassedTests, ValidationSuite.TotalTests);
    
    if (bLogDetailedResults)
    {
        for (const FQA_ValidationResult& Result : ValidationSuite.TestResults)
        {
            FString Status = Result.bPassed ? TEXT("PASS") : TEXT("FAIL");
            UE_LOG(LogTemp, Warning, TEXT("%s: %s (%.1fms)"), *Result.TestName, *Status, Result.ExecutionTimeMs);
            
            if (!Result.bPassed && !Result.ErrorMessage.IsEmpty())
            {
                UE_LOG(LogTemp, Error, TEXT("  Error: %s"), *Result.ErrorMessage);
            }
        }
    }
}

bool UQA_ValidationManager::SaveValidationReport(const FQA_ValidationSuite& ValidationSuite, const FString& FilePath)
{
    FString ReportContent;
    ReportContent += TEXT("QA VALIDATION REPORT\n");
    ReportContent += TEXT("====================\n\n");
    ReportContent += FString::Printf(TEXT("Execution Time: %s\n"), *ValidationSuite.ExecutionTime.ToString());
    ReportContent += FString::Printf(TEXT("Overall Score: %.1f%%\n"), ValidationSuite.OverallScore);
    ReportContent += FString::Printf(TEXT("Tests Passed: %d/%d\n\n"), ValidationSuite.PassedTests, ValidationSuite.TotalTests);
    
    ReportContent += TEXT("DETAILED RESULTS:\n");
    for (const FQA_ValidationResult& Result : ValidationSuite.TestResults)
    {
        FString Status = Result.bPassed ? TEXT("PASS") : TEXT("FAIL");
        ReportContent += FString::Printf(TEXT("- %s: %s (%.1fms)\n"), *Result.TestName, *Status, Result.ExecutionTimeMs);
        
        if (!Result.bPassed && !Result.ErrorMessage.IsEmpty())
        {
            ReportContent += FString::Printf(TEXT("  Error: %s\n"), *Result.ErrorMessage);
        }
    }
    
    return FFileHelper::SaveStringToFile(ReportContent, *FilePath);
}

FQA_ValidationResult UQA_ValidationManager::CreateTestResult(const FString& TestName, bool bPassed, const FString& ErrorMessage, float ExecutionTime)
{
    FQA_ValidationResult Result;
    Result.TestName = TestName;
    Result.bPassed = bPassed;
    Result.ErrorMessage = ErrorMessage;
    Result.ExecutionTimeMs = ExecutionTime;
    return Result;
}

TArray<AActor*> UQA_ValidationManager::GetActorsOfType(const FString& ActorTypeName)
{
    TArray<AActor*> FoundActors;
    UWorld* World = GetWorld();
    
    if (!World)
    {
        return FoundActors;
    }
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetClass()->GetName() == ActorTypeName)
        {
            FoundActors.Add(Actor);
        }
    }
    
    return FoundActors;
}

bool UQA_ValidationManager::IsClassLoadable(const FString& ClassPath)
{
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
    return LoadedClass != nullptr;
}
#include "Build_IntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectIterator.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"

UBuild_IntegrationValidator::UBuild_IntegrationValidator()
{
    // Initialize core module names for validation
    CoreModuleNames = {
        TEXT("TranspersonalGame"),
        TEXT("Core"),
        TEXT("Physics"), 
        TEXT("World"),
        TEXT("Environment"),
        TEXT("Character"),
        TEXT("Animation"),
        TEXT("NPC"),
        TEXT("Combat"),
        TEXT("Crowd"),
        TEXT("Quest"),
        TEXT("Narrative"),
        TEXT("Audio"),
        TEXT("VFX"),
        TEXT("QA")
    };

    // Set validation thresholds
    HealthyThreshold = 80.0f;
    WarningThreshold = 60.0f;
    bAutoValidateOnStartup = true;

    // Initialize report
    LastIntegrationReport = FBuild_IntegrationReport();
}

void UBuild_IntegrationValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationValidator: Subsystem initialized"));
    
    if (bAutoValidateOnStartup)
    {
        // Delay initial validation to allow other systems to initialize
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            ValidateIntegration();
        }, 2.0f, false);
    }
}

void UBuild_IntegrationValidator::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationValidator: Subsystem deinitialized"));
    Super::Deinitialize();
}

FBuild_IntegrationReport UBuild_IntegrationValidator::ValidateIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationValidator: Starting comprehensive integration validation"));

    // Reset report
    LastIntegrationReport = FBuild_IntegrationReport();
    LastIntegrationReport.ValidationTimestamp = FDateTime::Now();
    LastIntegrationReport.TotalModules = CoreModuleNames.Num();

    // Validate core modules
    ValidateCoreModules();

    // Validate actor integration
    ValidateActorIntegration();

    // Validate shared types
    ValidateSharedTypes();

    // Calculate overall status
    CalculateOverallStatus();

    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationValidator: Integration validation complete - Status: %s, Success Rate: %.1f%%"), 
        *UEnum::GetValueAsString(LastIntegrationReport.OverallStatus), 
        LastIntegrationReport.SuccessRate);

    return LastIntegrationReport;
}

FBuild_ModuleValidation UBuild_IntegrationValidator::ValidateModule(const FString& ModuleName)
{
    FBuild_ModuleValidation Validation;
    Validation.ModuleName = ModuleName;

    try
    {
        // Try to find classes from this module
        TArray<FString> TestClassPaths = {
            FString::Printf(TEXT("/Script/TranspersonalGame.%s_TestClass"), *ModuleName),
            FString::Printf(TEXT("/Script/TranspersonalGame.%sManager"), *ModuleName),
            FString::Printf(TEXT("/Script/TranspersonalGame.%sComponent"), *ModuleName),
            FString::Printf(TEXT("/Script/TranspersonalGame.%sSystem"), *ModuleName)
        };

        int32 FoundClasses = 0;
        for (const FString& ClassPath : TestClassPaths)
        {
            UClass* TestClass = LoadClass<UObject>(nullptr, *ClassPath);
            if (TestClass)
            {
                FoundClasses++;
                Validation.bIsLoaded = true;
            }
        }

        Validation.ClassCount = FoundClasses;
        Validation.bHasValidClasses = FoundClasses > 0;

        if (!Validation.bHasValidClasses)
        {
            Validation.ErrorMessage = TEXT("No loadable classes found for module");
        }
    }
    catch (...)
    {
        Validation.bIsLoaded = false;
        Validation.bHasValidClasses = false;
        Validation.ErrorMessage = TEXT("Exception during module validation");
    }

    return Validation;
}

EBuild_IntegrationStatus UBuild_IntegrationValidator::GetIntegrationStatus() const
{
    return LastIntegrationReport.OverallStatus;
}

void UBuild_IntegrationValidator::ForceRevalidation()
{
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationValidator: Force revalidation requested"));
    ValidateIntegration();
}

bool UBuild_IntegrationValidator::IsBuildHealthy() const
{
    return LastIntegrationReport.OverallStatus == EBuild_IntegrationStatus::Healthy ||
           LastIntegrationReport.SuccessRate >= HealthyThreshold;
}

void UBuild_IntegrationValidator::ValidateCoreModules()
{
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationValidator: Validating core modules"));

    LastIntegrationReport.ModuleValidations.Empty();
    int32 LoadedCount = 0;

    for (const FString& ModuleName : CoreModuleNames)
    {
        FBuild_ModuleValidation ModuleValidation = ValidateModule(ModuleName);
        LastIntegrationReport.ModuleValidations.Add(ModuleValidation);

        if (ModuleValidation.bIsLoaded)
        {
            LoadedCount++;
        }

        UE_LOG(LogTemp, Log, TEXT("Module %s: Loaded=%s, Classes=%d"), 
            *ModuleName, 
            ModuleValidation.bIsLoaded ? TEXT("Yes") : TEXT("No"),
            ModuleValidation.ClassCount);
    }

    LastIntegrationReport.LoadedModules = LoadedCount;
    LastIntegrationReport.SuccessRate = (float)LoadedCount / (float)LastIntegrationReport.TotalModules * 100.0f;
}

void UBuild_IntegrationValidator::ValidateActorIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationValidator: Validating actor integration"));

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationValidator: No world available for actor validation"));
        return;
    }

    int32 TotalActors = 0;
    int32 CustomActors = 0;

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            TotalActors++;
            
            FString ClassName = Actor->GetClass()->GetName();
            if (ClassName.Contains(TEXT("Transpersonal")))
            {
                CustomActors++;
            }
        }
    }

    LastIntegrationReport.TotalActorsInLevel = TotalActors;
    LastIntegrationReport.ActiveCustomActors = CustomActors;

    UE_LOG(LogTemp, Log, TEXT("Actor Integration: Total=%d, Custom=%d"), TotalActors, CustomActors);
}

void UBuild_IntegrationValidator::ValidateSharedTypes()
{
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationValidator: Validating shared types integration"));

    // Try to validate SharedTypes integration
    UClass* SharedTypesClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.SharedTypes"));
    if (SharedTypesClass)
    {
        UE_LOG(LogTemp, Log, TEXT("SharedTypes: Successfully validated"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SharedTypes: Validation failed - may need compilation"));
    }
}

void UBuild_IntegrationValidator::CalculateOverallStatus()
{
    float SuccessRate = LastIntegrationReport.SuccessRate;

    if (SuccessRate >= HealthyThreshold)
    {
        LastIntegrationReport.OverallStatus = EBuild_IntegrationStatus::Healthy;
    }
    else if (SuccessRate >= WarningThreshold)
    {
        LastIntegrationReport.OverallStatus = EBuild_IntegrationStatus::Warning;
    }
    else if (SuccessRate > 0.0f)
    {
        LastIntegrationReport.OverallStatus = EBuild_IntegrationStatus::Critical;
    }
    else
    {
        LastIntegrationReport.OverallStatus = EBuild_IntegrationStatus::Failed;
    }

    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationValidator: Overall status calculated as %s"), 
        *UEnum::GetValueAsString(LastIntegrationReport.OverallStatus));
}
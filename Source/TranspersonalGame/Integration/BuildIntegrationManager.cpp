#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Materials/Material.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

ABuild_IntegrationMonitor::ABuild_IntegrationMonitor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create status mesh component
    StatusMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StatusMesh"));
    StatusMesh->SetupAttachment(RootComponent);

    // Set default mesh to cube
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        StatusMesh->SetStaticMesh(CubeMeshAsset.Object);
    }

    // Set default material
    static ConstructorHelpers::FObjectFinder<UMaterial> DefaultMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    if (DefaultMaterial.Succeeded())
    {
        StatusMesh->SetMaterial(0, DefaultMaterial.Object);
    }

    // Initialize status
    CurrentStatus = EBuild_BuildStatus::Unknown;
    
    // Set actor properties
    SetActorScale3D(FVector(2.0f, 2.0f, 2.0f));
}

void ABuild_IntegrationMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize with current status
    UpdateVisualStatus();
    
    UE_LOG(LogTemp, Log, TEXT("Build Integration Monitor initialized"));
}

void ABuild_IntegrationMonitor::UpdateBuildStatus(EBuild_BuildStatus NewStatus)
{
    if (CurrentStatus != NewStatus)
    {
        CurrentStatus = NewStatus;
        UpdateVisualStatus();
        
        UE_LOG(LogTemp, Log, TEXT("Build status updated to: %d"), (int32)NewStatus);
    }
}

void ABuild_IntegrationMonitor::SetIntegrationReport(const FBuild_IntegrationReport& Report)
{
    LastReport = Report;
    UpdateBuildStatus(Report.OverallStatus);
}

void ABuild_IntegrationMonitor::UpdateVisualStatus()
{
    if (!StatusMesh)
        return;

    // Change material color based on status
    FLinearColor StatusColor = FLinearColor::White;
    
    switch (CurrentStatus)
    {
        case EBuild_BuildStatus::Success:
            StatusColor = FLinearColor::Green;
            break;
        case EBuild_BuildStatus::Failed:
            StatusColor = FLinearColor::Red;
            break;
        case EBuild_BuildStatus::Warning:
            StatusColor = FLinearColor::Yellow;
            break;
        case EBuild_BuildStatus::Compiling:
        case EBuild_BuildStatus::Testing:
            StatusColor = FLinearColor::Blue;
            break;
        case EBuild_BuildStatus::Initializing:
            StatusColor = FLinearColor(1.0f, 0.5f, 0.0f); // Orange
            break;
        default:
            StatusColor = FLinearColor::Gray;
            break;
    }

    // Create dynamic material instance to change color
    if (UMaterialInterface* CurrentMaterial = StatusMesh->GetMaterial(0))
    {
        UMaterialInstanceDynamic* DynamicMaterial = StatusMesh->CreateDynamicMaterialInstance(0, CurrentMaterial);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), StatusColor);
        }
    }
}

// Build Integration Subsystem Implementation

void UBuild_IntegrationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeTestParameters();
    
    // Initialize current report
    CurrentReport.CycleId = TEXT("PROD_CYCLE_014");
    CurrentReport.Timestamp = FDateTime::Now();
    CurrentReport.OverallStatus = EBuild_BuildStatus::Initializing;
    
    UE_LOG(LogTemp, Log, TEXT("Build Integration Subsystem initialized"));
}

void UBuild_IntegrationSubsystem::Deinitialize()
{
    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("Build Integration Subsystem deinitialized"));
}

void UBuild_IntegrationSubsystem::InitializeTestParameters()
{
    // Define modules to test
    ModulesToTest = {
        TEXT("Core Systems"),
        TEXT("World Generation"),
        TEXT("AI & Crowd"),
        TEXT("Character Systems"),
        TEXT("Environment"),
        TEXT("Physics"),
        TEXT("Audio"),
        TEXT("VFX"),
        TEXT("Quest & Narrative")
    };

    // Define critical classes that must load
    CriticalClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager"),
        TEXT("/Script/TranspersonalGame.CrowdSimulationManager")
    };
}

void UBuild_IntegrationSubsystem::RunIntegrationTests()
{
    UE_LOG(LogTemp, Log, TEXT("Starting integration tests..."));
    
    CurrentReport.Timestamp = FDateTime::Now();
    CurrentReport.OverallStatus = EBuild_BuildStatus::Testing;
    CurrentReport.ModuleResults.Empty();
    CurrentReport.CompilationErrors.Empty();
    CurrentReport.Recommendations.Empty();

    // Test module loading
    TestModuleLoading();
    
    // Update overall status based on results
    UpdateOverallStatus();
    
    // Generate recommendations
    GenerateIntegrationReport();
    
    UE_LOG(LogTemp, Log, TEXT("Integration tests completed with status: %d"), (int32)CurrentReport.OverallStatus);
}

void UBuild_IntegrationSubsystem::TestModuleLoading()
{
    // Test core systems
    TArray<FString> CoreClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter")
    };
    FBuild_ModuleTestResult CoreResult = TestModule(TEXT("Core Systems"), CoreClasses);
    CurrentReport.ModuleResults.Add(CoreResult);

    // Test world generation
    TArray<FString> WorldGenClasses = {
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager")
    };
    FBuild_ModuleTestResult WorldGenResult = TestModule(TEXT("World Generation"), WorldGenClasses);
    CurrentReport.ModuleResults.Add(WorldGenResult);

    // Test AI & Crowd
    TArray<FString> AIClasses = {
        TEXT("/Script/TranspersonalGame.CrowdSimulationManager")
    };
    FBuild_ModuleTestResult AIResult = TestModule(TEXT("AI & Crowd"), AIClasses);
    CurrentReport.ModuleResults.Add(AIResult);
}

FBuild_ModuleTestResult UBuild_IntegrationSubsystem::TestModule(const FString& ModuleName, const TArray<FString>& ClassPaths)
{
    FBuild_ModuleTestResult Result;
    Result.ModuleName = ModuleName;
    Result.TotalClasses = ClassPaths.Num();
    Result.ClassesLoaded = 0;

    for (const FString& ClassPath : ClassPaths)
    {
        FString Error;
        if (TestClassLoading(ClassPath, Error))
        {
            Result.ClassesLoaded++;
        }
        else
        {
            Result.ErrorMessages.Add(FString::Printf(TEXT("%s: %s"), *ClassPath, *Error));
        }
    }

    // Determine status
    if (Result.ClassesLoaded == Result.TotalClasses)
    {
        Result.Status = EBuild_BuildStatus::Success;
    }
    else if (Result.ClassesLoaded > 0)
    {
        Result.Status = EBuild_BuildStatus::Warning;
    }
    else
    {
        Result.Status = EBuild_BuildStatus::Failed;
    }

    return Result;
}

bool UBuild_IntegrationSubsystem::TestClassLoading(const FString& ClassPath, FString& OutError)
{
    try
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        if (LoadedClass)
        {
            // Try to get CDO
            UObject* CDO = LoadedClass->GetDefaultObject();
            if (CDO)
            {
                return true;
            }
            else
            {
                OutError = TEXT("CDO not available");
                return false;
            }
        }
        else
        {
            OutError = TEXT("Class not found");
            return false;
        }
    }
    catch (...)
    {
        OutError = TEXT("Exception during loading");
        return false;
    }
}

void UBuild_IntegrationSubsystem::UpdateOverallStatus()
{
    int32 SuccessCount = 0;
    int32 WarningCount = 0;
    int32 FailedCount = 0;

    for (const FBuild_ModuleTestResult& Result : CurrentReport.ModuleResults)
    {
        switch (Result.Status)
        {
            case EBuild_BuildStatus::Success:
                SuccessCount++;
                break;
            case EBuild_BuildStatus::Warning:
                WarningCount++;
                break;
            case EBuild_BuildStatus::Failed:
                FailedCount++;
                break;
        }
    }

    if (FailedCount > 0)
    {
        CurrentReport.OverallStatus = EBuild_BuildStatus::Failed;
    }
    else if (WarningCount > 0)
    {
        CurrentReport.OverallStatus = EBuild_BuildStatus::Warning;
    }
    else if (SuccessCount > 0)
    {
        CurrentReport.OverallStatus = EBuild_BuildStatus::Success;
    }
    else
    {
        CurrentReport.OverallStatus = EBuild_BuildStatus::Unknown;
    }
}

void UBuild_IntegrationSubsystem::GenerateIntegrationReport()
{
    CurrentReport.Recommendations.Empty();

    // Analyze results and generate recommendations
    for (const FBuild_ModuleTestResult& Result : CurrentReport.ModuleResults)
    {
        if (Result.Status == EBuild_BuildStatus::Failed)
        {
            CurrentReport.Recommendations.Add(
                FString::Printf(TEXT("CRITICAL: %s module failed to load - requires immediate attention"), *Result.ModuleName)
            );
        }
        else if (Result.Status == EBuild_BuildStatus::Warning)
        {
            CurrentReport.Recommendations.Add(
                FString::Printf(TEXT("WARNING: %s module partially loaded (%d/%d classes)"), 
                    *Result.ModuleName, Result.ClassesLoaded, Result.TotalClasses)
            );
        }
    }

    // Add general recommendations
    if (CurrentReport.OverallStatus == EBuild_BuildStatus::Success)
    {
        CurrentReport.Recommendations.Add(TEXT("All modules loaded successfully - ready for next development phase"));
    }
    else
    {
        CurrentReport.Recommendations.Add(TEXT("Build integration issues detected - review module dependencies"));
    }
}

bool UBuild_IntegrationSubsystem::SaveReportToFile(const FString& FilePath)
{
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    
    JsonObject->SetStringField(TEXT("CycleId"), CurrentReport.CycleId);
    JsonObject->SetStringField(TEXT("Timestamp"), CurrentReport.Timestamp.ToString());
    JsonObject->SetNumberField(TEXT("OverallStatus"), (int32)CurrentReport.OverallStatus);
    
    // Add module results
    TArray<TSharedPtr<FJsonValue>> ModuleArray;
    for (const FBuild_ModuleTestResult& Result : CurrentReport.ModuleResults)
    {
        TSharedPtr<FJsonObject> ModuleObj = MakeShareable(new FJsonObject);
        ModuleObj->SetStringField(TEXT("ModuleName"), Result.ModuleName);
        ModuleObj->SetNumberField(TEXT("ClassesLoaded"), Result.ClassesLoaded);
        ModuleObj->SetNumberField(TEXT("TotalClasses"), Result.TotalClasses);
        ModuleObj->SetNumberField(TEXT("Status"), (int32)Result.Status);
        
        TArray<TSharedPtr<FJsonValue>> ErrorArray;
        for (const FString& Error : Result.ErrorMessages)
        {
            ErrorArray.Add(MakeShareable(new FJsonValueString(Error)));
        }
        ModuleObj->SetArrayField(TEXT("ErrorMessages"), ErrorArray);
        
        ModuleArray.Add(MakeShareable(new FJsonValueObject(ModuleObj)));
    }
    JsonObject->SetArrayField(TEXT("ModuleResults"), ModuleArray);
    
    // Add recommendations
    TArray<TSharedPtr<FJsonValue>> RecommendationArray;
    for (const FString& Recommendation : CurrentReport.Recommendations)
    {
        RecommendationArray.Add(MakeShareable(new FJsonValueString(Recommendation)));
    }
    JsonObject->SetArrayField(TEXT("Recommendations"), RecommendationArray);
    
    // Serialize to string
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    
    // Save to file
    return FFileHelper::SaveStringToFile(OutputString, *FilePath);
}

bool UBuild_IntegrationSubsystem::LoadReportFromFile(const FString& FilePath)
{
    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
    {
        return false;
    }
    
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    
    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        CurrentReport.CycleId = JsonObject->GetStringField(TEXT("CycleId"));
        
        FString TimestampString = JsonObject->GetStringField(TEXT("Timestamp"));
        FDateTime::Parse(TimestampString, CurrentReport.Timestamp);
        
        CurrentReport.OverallStatus = (EBuild_BuildStatus)JsonObject->GetIntegerField(TEXT("OverallStatus"));
        
        // Load module results
        CurrentReport.ModuleResults.Empty();
        const TArray<TSharedPtr<FJsonValue>>* ModuleArray;
        if (JsonObject->TryGetArrayField(TEXT("ModuleResults"), ModuleArray))
        {
            for (const TSharedPtr<FJsonValue>& ModuleValue : *ModuleArray)
            {
                TSharedPtr<FJsonObject> ModuleObj = ModuleValue->AsObject();
                if (ModuleObj.IsValid())
                {
                    FBuild_ModuleTestResult Result;
                    Result.ModuleName = ModuleObj->GetStringField(TEXT("ModuleName"));
                    Result.ClassesLoaded = ModuleObj->GetIntegerField(TEXT("ClassesLoaded"));
                    Result.TotalClasses = ModuleObj->GetIntegerField(TEXT("TotalClasses"));
                    Result.Status = (EBuild_BuildStatus)ModuleObj->GetIntegerField(TEXT("Status"));
                    
                    const TArray<TSharedPtr<FJsonValue>>* ErrorArray;
                    if (ModuleObj->TryGetArrayField(TEXT("ErrorMessages"), ErrorArray))
                    {
                        for (const TSharedPtr<FJsonValue>& ErrorValue : *ErrorArray)
                        {
                            Result.ErrorMessages.Add(ErrorValue->AsString());
                        }
                    }
                    
                    CurrentReport.ModuleResults.Add(Result);
                }
            }
        }
        
        // Load recommendations
        CurrentReport.Recommendations.Empty();
        const TArray<TSharedPtr<FJsonValue>>* RecommendationArray;
        if (JsonObject->TryGetArrayField(TEXT("Recommendations"), RecommendationArray))
        {
            for (const TSharedPtr<FJsonValue>& RecommendationValue : *RecommendationArray)
            {
                CurrentReport.Recommendations.Add(RecommendationValue->AsString());
            }
        }
        
        return true;
    }
    
    return false;
}
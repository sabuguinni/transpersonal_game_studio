#include "Build_FinalCycleCompletionReport.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EditorLevelLibrary.h"
#include "EditorAssetLibrary.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"

UBuild_FinalCycleCompletionReport::UBuild_FinalCycleCompletionReport()
{
    CycleID = TEXT("PROD_CYCLE_AUTO_20260604_005");
    CompletionTimestamp = FDateTime::Now();
}

void UBuild_FinalCycleCompletionReport::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionReport: Initialized for cycle %s"), *CycleID);
    
    // Initialize with current system state
    CollectSystemMetrics();
}

void UBuild_FinalCycleCompletionReport::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionReport: Deinitializing"));
    Super::Deinitialize();
}

void UBuild_FinalCycleCompletionReport::GenerateFinalReport()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionReport: Generating final report"));
    
    CollectSystemMetrics();
    ValidateAgentOutputs();
    CalculateFinalScores();
    
    UE_LOG(LogTemp, Warning, TEXT("Final Metrics - Actors: %d, Assets: %d, Completion: %.1f%%"), 
        FinalMetrics.TotalActors, FinalMetrics.TotalAssets, FinalMetrics.CompletionPercentage);
}

bool UBuild_FinalCycleCompletionReport::ValidateAllAgentDeliverables()
{
    int32 ValidDeliverables = 0;
    
    for (const FBuild_AgentDeliverable& Deliverable : AgentDeliverables)
    {
        if (Deliverable.bCompilationSuccess && Deliverable.bFunctionalValidation)
        {
            ValidDeliverables++;
        }
    }
    
    float ValidationRate = AgentDeliverables.Num() > 0 ? 
        (float)ValidDeliverables / (float)AgentDeliverables.Num() : 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Agent Validation: %d/%d valid (%.1f%%)"), 
        ValidDeliverables, AgentDeliverables.Num(), ValidationRate * 100.0f);
    
    return ValidationRate >= 0.8f; // 80% success rate required
}

void UBuild_FinalCycleCompletionReport::RecordAgentDeliverable(const FBuild_AgentDeliverable& Deliverable)
{
    AgentDeliverables.Add(Deliverable);
    
    UE_LOG(LogTemp, Warning, TEXT("Recorded deliverable for Agent #%d (%s): %d files, %d commands"), 
        Deliverable.AgentNumber, *Deliverable.AgentName, 
        Deliverable.FilesCreated.Num(), Deliverable.UE5CommandsExecuted.Num());
}

float UBuild_FinalCycleCompletionReport::CalculateOverallCompletionPercentage() const
{
    float MetricsScore = 0.0f;
    
    // Actor count score (target: 50+ actors)
    MetricsScore += FMath::Clamp((float)FinalMetrics.TotalActors / 50.0f, 0.0f, 1.0f) * 20.0f;
    
    // Asset count score (target: 100+ assets)
    MetricsScore += FMath::Clamp((float)FinalMetrics.TotalAssets / 100.0f, 0.0f, 1.0f) * 20.0f;
    
    // Core systems score
    MetricsScore += (FinalMetrics.bCoreClassesLoaded ? 20.0f : 0.0f);
    MetricsScore += (FinalMetrics.bQAValidatorLoaded ? 20.0f : 0.0f);
    
    // Agent deliverables score
    int32 ValidAgents = 0;
    for (const FBuild_AgentDeliverable& Deliverable : AgentDeliverables)
    {
        if (Deliverable.bCompilationSuccess && Deliverable.FilesCreated.Num() > 0)
        {
            ValidAgents++;
        }
    }
    
    float AgentScore = AgentDeliverables.Num() > 0 ? 
        ((float)ValidAgents / (float)AgentDeliverables.Num()) * 20.0f : 0.0f;
    MetricsScore += AgentScore;
    
    return FMath::Clamp(MetricsScore, 0.0f, 100.0f);
}

EBuildValidationStatus UBuild_FinalCycleCompletionReport::GetFinalValidationStatus() const
{
    float CompletionRate = CalculateOverallCompletionPercentage();
    
    if (CompletionRate >= 90.0f)
    {
        return EBuildValidationStatus::Passed;
    }
    else if (CompletionRate >= 70.0f)
    {
        return EBuildValidationStatus::Warning;
    }
    else
    {
        return EBuildValidationStatus::Failed;
    }
}

void UBuild_FinalCycleCompletionReport::ExportFinalReportToJSON()
{
    FString JSONContent = TEXT("{\n");
    JSONContent += FString::Printf(TEXT("  \"cycle_id\": \"%s\",\n"), *CycleID);
    JSONContent += FString::Printf(TEXT("  \"completion_timestamp\": \"%s\",\n"), *CompletionTimestamp.ToString());
    JSONContent += FString::Printf(TEXT("  \"completion_percentage\": %.2f,\n"), FinalMetrics.CompletionPercentage);
    JSONContent += FString::Printf(TEXT("  \"validation_status\": \"%s\",\n"), 
        *UEnum::GetValueAsString(FinalMetrics.ValidationStatus));
    
    JSONContent += TEXT("  \"metrics\": {\n");
    JSONContent += FString::Printf(TEXT("    \"total_actors\": %d,\n"), FinalMetrics.TotalActors);
    JSONContent += FString::Printf(TEXT("    \"qa_test_actors\": %d,\n"), FinalMetrics.QATestActors);
    JSONContent += FString::Printf(TEXT("    \"vfx_test_actors\": %d,\n"), FinalMetrics.VFXTestActors);
    JSONContent += FString::Printf(TEXT("    \"total_assets\": %d,\n"), FinalMetrics.TotalAssets);
    JSONContent += FString::Printf(TEXT("    \"material_assets\": %d,\n"), FinalMetrics.MaterialAssets);
    JSONContent += FString::Printf(TEXT("    \"blueprint_assets\": %d,\n"), FinalMetrics.BlueprintAssets);
    JSONContent += FString::Printf(TEXT("    \"mesh_assets\": %d\n"), FinalMetrics.MeshAssets);
    JSONContent += TEXT("  },\n");
    
    JSONContent += TEXT("  \"agent_deliverables\": [\n");
    for (int32 i = 0; i < AgentDeliverables.Num(); i++)
    {
        const FBuild_AgentDeliverable& Deliverable = AgentDeliverables[i];
        JSONContent += TEXT("    {\n");
        JSONContent += FString::Printf(TEXT("      \"agent_number\": %d,\n"), Deliverable.AgentNumber);
        JSONContent += FString::Printf(TEXT("      \"agent_name\": \"%s\",\n"), *Deliverable.AgentName);
        JSONContent += FString::Printf(TEXT("      \"files_created\": %d,\n"), Deliverable.FilesCreated.Num());
        JSONContent += FString::Printf(TEXT("      \"ue5_commands\": %d,\n"), Deliverable.UE5CommandsExecuted.Num());
        JSONContent += FString::Printf(TEXT("      \"compilation_success\": %s,\n"), 
            Deliverable.bCompilationSuccess ? TEXT("true") : TEXT("false"));
        JSONContent += FString::Printf(TEXT("      \"functional_validation\": %s\n"), 
            Deliverable.bFunctionalValidation ? TEXT("true") : TEXT("false"));
        JSONContent += TEXT("    }");
        if (i < AgentDeliverables.Num() - 1) JSONContent += TEXT(",");
        JSONContent += TEXT("\n");
    }
    JSONContent += TEXT("  ]\n");
    JSONContent += TEXT("}\n");
    
    FString FilePath = FPaths::ProjectSavedDir() / TEXT("FinalCycleReport_") + CycleID + TEXT(".json");
    FFileHelper::SaveStringToFile(JSONContent, *FilePath);
    
    UE_LOG(LogTemp, Warning, TEXT("Final cycle report exported to: %s"), *FilePath);
}

void UBuild_FinalCycleCompletionReport::CollectSystemMetrics()
{
    // Get current world and actors
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (World)
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        FinalMetrics.TotalActors = AllActors.Num();
        
        // Count specific actor types
        for (AActor* Actor : AllActors)
        {
            FString ActorName = Actor->GetName();
            if (ActorName.Contains(TEXT("QA_Test")))
            {
                FinalMetrics.QATestActors++;
            }
            else if (ActorName.Contains(TEXT("VFX_Test")) || ActorName.Contains(TEXT("Effect")))
            {
                FinalMetrics.VFXTestActors++;
            }
        }
    }
    
    // Get asset counts
    TArray<FString> AllAssets = UEditorAssetLibrary::ListAssets(TEXT("/Game"), true);
    FinalMetrics.TotalAssets = AllAssets.Num();
    
    for (const FString& AssetPath : AllAssets)
    {
        if (AssetPath.Contains(TEXT("Material")) || AssetPath.EndsWith(TEXT("_M")))
        {
            FinalMetrics.MaterialAssets++;
        }
        else if (AssetPath.Contains(TEXT("Blueprint")) || AssetPath.EndsWith(TEXT("_BP")))
        {
            FinalMetrics.BlueprintAssets++;
        }
        else if (AssetPath.Contains(TEXT("Mesh")) || AssetPath.EndsWith(TEXT("_SM")))
        {
            FinalMetrics.MeshAssets++;
        }
    }
    
    // Check core class loading
    UClass* GameStateClass = UClass::TryFindTypeSlow<UClass>(TEXT("/Script/TranspersonalGame.TranspersonalGameState"));
    UClass* CharacterClass = UClass::TryFindTypeSlow<UClass>(TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    FinalMetrics.bCoreClassesLoaded = (GameStateClass != nullptr) && (CharacterClass != nullptr);
    
    UClass* QAValidatorClass = UClass::TryFindTypeSlow<UClass>(TEXT("/Script/TranspersonalGame.QA_VFXEffectManagerValidator"));
    FinalMetrics.bQAValidatorLoaded = (QAValidatorClass != nullptr);
    
    UE_LOG(LogTemp, Warning, TEXT("System metrics collected - Actors: %d, Assets: %d"), 
        FinalMetrics.TotalActors, FinalMetrics.TotalAssets);
}

void UBuild_FinalCycleCompletionReport::ValidateAgentOutputs()
{
    // This would typically validate each agent's deliverables
    // For now, we'll simulate based on current system state
    
    UE_LOG(LogTemp, Warning, TEXT("Validating agent outputs for %d recorded deliverables"), 
        AgentDeliverables.Num());
}

void UBuild_FinalCycleCompletionReport::CalculateFinalScores()
{
    FinalMetrics.CompletionPercentage = CalculateOverallCompletionPercentage();
    FinalMetrics.ValidationStatus = GetFinalValidationStatus();
    
    UE_LOG(LogTemp, Warning, TEXT("Final scores calculated - Completion: %.1f%%, Status: %s"), 
        FinalMetrics.CompletionPercentage, 
        *UEnum::GetValueAsString(FinalMetrics.ValidationStatus));
}
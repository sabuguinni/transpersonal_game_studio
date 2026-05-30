#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_ArchitecturalStandards.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleStandard
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Standard")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Standard")
    int32 MinimumFilesRequired;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Standard")
    int32 MinimumUE5Commands;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Standard")
    bool bRequiresBiomeDistribution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Standard")
    TArray<FString> RequiredInterfaces;

    FEng_ModuleStandard()
    {
        ModuleName = TEXT("Unknown");
        MinimumFilesRequired = 2;
        MinimumUE5Commands = 1;
        bRequiresBiomeDistribution = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ComplianceReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compliance")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compliance")
    int32 FilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compliance")
    int32 UE5CommandsExecuted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compliance")
    bool bMeetsStandards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compliance")
    TArray<FString> ViolationReasons;

    FEng_ComplianceReport()
    {
        AgentName = TEXT("Unknown");
        FilesCreated = 0;
        UE5CommandsExecuted = 0;
        bMeetsStandards = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ArchitecturalStandards : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_ArchitecturalStandards();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Standards Management
    UFUNCTION(BlueprintCallable, Category = "Architectural Standards")
    void InitializeStandards();

    UFUNCTION(BlueprintCallable, Category = "Architectural Standards")
    FEng_ModuleStandard GetStandardForAgent(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Architectural Standards")
    void SetStandardForAgent(int32 AgentID, const FEng_ModuleStandard& Standard);

    // Compliance Validation
    UFUNCTION(BlueprintCallable, Category = "Architectural Standards")
    FEng_ComplianceReport ValidateAgentCompliance(int32 AgentID, int32 FilesCreated, int32 UE5Commands) const;

    UFUNCTION(BlueprintCallable, Category = "Architectural Standards")
    bool IsAgentCompliant(int32 AgentID, int32 FilesCreated, int32 UE5Commands) const;

    UFUNCTION(BlueprintCallable, Category = "Architectural Standards")
    TArray<FString> GetComplianceViolations(int32 AgentID, int32 FilesCreated, int32 UE5Commands) const;

    // Biome Distribution Standards
    UFUNCTION(BlueprintCallable, Category = "Architectural Standards")
    bool ValidateBiomeDistribution(const TArray<FVector>& SpawnLocations) const;

    UFUNCTION(BlueprintCallable, Category = "Architectural Standards")
    TArray<FVector> GetRequiredBiomeDistribution(int32 NumActors) const;

    // File Standards
    UFUNCTION(BlueprintCallable, Category = "Architectural Standards")
    bool ValidateFileNaming(const FString& FileName, int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Architectural Standards")
    FString GetRequiredFilePrefix(int32 AgentID) const;

    // Performance Standards
    UFUNCTION(BlueprintCallable, Category = "Architectural Standards")
    bool ValidatePerformanceRequirements(float FrameTime, int32 ActorCount) const;

    UFUNCTION(BlueprintCallable, Category = "Architectural Standards")
    void EnforcePerformanceLimits();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Standards", meta = (AllowPrivateAccess = "true"))
    TMap<int32, FEng_ModuleStandard> AgentStandards;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Standards", meta = (AllowPrivateAccess = "true"))
    float MaxFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Standards", meta = (AllowPrivateAccess = "true"))
    int32 MaxActorsPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Standards", meta = (AllowPrivateAccess = "true"))
    TArray<FVector> BiomeCenters;

private:
    void SetupAgentStandards();
    bool ValidateAgentFileCount(int32 AgentID, int32 FilesCreated) const;
    bool ValidateAgentCommandCount(int32 AgentID, int32 UE5Commands) const;
};
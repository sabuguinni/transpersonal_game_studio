#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "EngArch_ModuleRegistry.generated.h"

/**
 * Module Registry - Centralized type registration system to prevent
 * compilation conflicts between the 19 agents. Enforces unique naming
 * and provides shared type discovery across all game modules.
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_RegisteredType
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
    FString TypeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
    FString AgentOwner;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
    FString TypeCategory; // UCLASS, USTRUCT, UENUM, UFUNCTION

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
    FString FilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
    bool bIsSharedType = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
    float RegistrationTime = 0.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_AgentModule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString ModulePrefix;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    TArray<FString> RegisteredTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    bool bCanRegisterTypes = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 TypeCount = 0;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ConflictReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conflict")
    FString ConflictingTypeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conflict")
    FString FirstAgent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conflict")
    FString SecondAgent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conflict")
    FString ConflictType; // "DUPLICATE_NAME", "CIRCULAR_DEPENDENCY", "MISSING_DEPENDENCY"

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conflict")
    FString Resolution;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngArch_ModuleRegistry : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngArch_ModuleRegistry();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent Registration
    UFUNCTION(BlueprintCallable, Category = "Registry")
    void RegisterAgent(int32 AgentNumber, const FString& AgentName, const FString& ModulePrefix);

    UFUNCTION(BlueprintCallable, Category = "Registry")
    bool IsAgentRegistered(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Registry")
    FEng_AgentModule GetAgentModule(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Registry")
    TArray<FEng_AgentModule> GetAllAgentModules() const;

    // Type Registration
    UFUNCTION(BlueprintCallable, Category = "Types")
    bool RegisterType(const FString& TypeName, const FString& ModuleName, int32 AgentNumber, 
                     const FString& TypeCategory, const FString& FilePath, bool bIsShared = false);

    UFUNCTION(BlueprintCallable, Category = "Types")
    bool IsTypeRegistered(const FString& TypeName) const;

    UFUNCTION(BlueprintCallable, Category = "Types")
    FEng_RegisteredType GetTypeInfo(const FString& TypeName) const;

    UFUNCTION(BlueprintCallable, Category = "Types")
    TArray<FEng_RegisteredType> GetTypesForAgent(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Types")
    TArray<FEng_RegisteredType> GetSharedTypes() const;

    // Conflict Detection
    UFUNCTION(BlueprintCallable, Category = "Validation")
    TArray<FEng_ConflictReport> DetectConflicts() const;

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateTypeRegistration(const FString& TypeName, int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateAgentDependencies(int32 AgentNumber) const;

    // Shared Types Management
    UFUNCTION(BlueprintCallable, Category = "SharedTypes")
    void RegisterSharedType(const FString& TypeName, const FString& TypeCategory);

    UFUNCTION(BlueprintCallable, Category = "SharedTypes")
    bool IsSharedType(const FString& TypeName) const;

    UFUNCTION(BlueprintCallable, Category = "SharedTypes")
    void GenerateSharedTypesHeader();

    // Build Integration
    UFUNCTION(BlueprintCallable, Category = "Build")
    TArray<FString> GetRequiredIncludes(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Build")
    TArray<FString> GetModuleDependencies(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Build")
    void ValidateModuleBuildFiles();

    // Naming Conventions
    UFUNCTION(BlueprintCallable, Category = "Naming")
    FString GenerateUniqueTypeName(const FString& BaseName, int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Naming")
    bool ValidateNamingConvention(const FString& TypeName, int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Naming")
    FString GetAgentPrefix(int32 AgentNumber) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Registry")
    TArray<FEng_AgentModule> RegisteredAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Registry")
    TArray<FEng_RegisteredType> RegisteredTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conflicts")
    TArray<FEng_ConflictReport> DetectedConflicts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SharedTypes")
    TArray<FString> SharedTypeNames;

private:
    void InitializeAgentPrefixes();
    void RegisterCoreTypes();
    void ValidateExistingTypes();
    FString GetStandardPrefix(int32 AgentNumber) const;
    bool CheckCircularDependency(int32 AgentNumber, TSet<int32>& VisitedAgents) const;
};
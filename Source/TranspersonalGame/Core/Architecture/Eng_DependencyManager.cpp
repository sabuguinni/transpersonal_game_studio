#include "Eng_DependencyManager.h"
#include "Engine/Engine.h"
#include "UObject/UObjectGlobals.h"
#include "Modules/ModuleManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManager.h"

void UEng_DependencyManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    Dependencies.Empty();
    bValidationComplete = false;
    
    UE_LOG(LogTemp, Log, TEXT("Eng_DependencyManager initialized"));
    
    RegisterCoreDependencies();
    RegisterModuleDependencies();
    RegisterAssetDependencies();
    BuildDependencyGraph();
}

void UEng_DependencyManager::RegisterDependency(const FString& Name, EEng_DependencyType Type, const FString& Path)
{
    FEng_DependencyInfo NewDependency;
    NewDependency.Name = Name;
    NewDependency.Type = Type;
    NewDependency.Path = Path;
    NewDependency.Status = EEng_DependencyStatus::Unknown;
    
    Dependencies.Add(Name, NewDependency);
    
    UE_LOG(LogTemp, Log, TEXT("Registered dependency: %s (%s)"), *Name, *Path);
}

void UEng_DependencyManager::AddDependencyRelation(const FString& Dependent, const FString& Dependency)
{
    if (FEng_DependencyInfo* DependentInfo = Dependencies.Find(Dependent))
    {
        DependentInfo->Requires.AddUnique(Dependency);
    }
    
    if (FEng_DependencyInfo* DependencyInfo = Dependencies.Find(Dependency))
    {
        DependencyInfo->RequiredBy.AddUnique(Dependent);
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Added relation: %s requires %s"), *Dependent, *Dependency);
}

bool UEng_DependencyManager::CheckDependency(const FString& Name)
{
    FEng_DependencyInfo* DependencyInfo = Dependencies.Find(Name);
    if (!DependencyInfo)
    {
        return false;
    }
    
    EEng_DependencyStatus Status = CheckDependencyStatus(*DependencyInfo);
    DependencyInfo->Status = Status;
    
    return Status == EEng_DependencyStatus::Available;
}

TArray<FEng_DependencyInfo> UEng_DependencyManager::GetAllDependencies() const
{
    TArray<FEng_DependencyInfo> AllDependencies;
    Dependencies.GenerateValueArray(AllDependencies);
    return AllDependencies;
}

TArray<FString> UEng_DependencyManager::GetMissingDependencies() const
{
    TArray<FString> Missing;
    
    for (const auto& Pair : Dependencies)
    {
        if (Pair.Value.Status == EEng_DependencyStatus::Missing || 
            Pair.Value.Status == EEng_DependencyStatus::Failed)
        {
            Missing.Add(Pair.Key);
        }
    }
    
    return Missing;
}

bool UEng_DependencyManager::ValidateAllDependencies()
{
    bool bAllValid = true;
    int32 ValidCount = 0;
    int32 InvalidCount = 0;
    
    for (auto& Pair : Dependencies)
    {
        EEng_DependencyStatus Status = CheckDependencyStatus(Pair.Value);
        Pair.Value.Status = Status;
        
        if (Status == EEng_DependencyStatus::Available)
        {
            ValidCount++;
        }
        else
        {
            InvalidCount++;
            bAllValid = false;
        }
    }
    
    bValidationComplete = true;
    
    UE_LOG(LogTemp, Log, TEXT("Dependency validation complete: %d valid, %d invalid"), ValidCount, InvalidCount);
    
    return bAllValid;
}

TArray<FString> UEng_DependencyManager::GetDependencyChain(const FString& Name) const
{
    TArray<FString> Chain;
    TSet<FString> Visited;
    
    TFunction<void(const FString&)> BuildChain = [&](const FString& CurrentName)
    {
        if (Visited.Contains(CurrentName))
        {
            return; // Circular dependency detected
        }
        
        Visited.Add(CurrentName);
        Chain.Add(CurrentName);
        
        const FEng_DependencyInfo* Info = Dependencies.Find(CurrentName);
        if (Info)
        {
            for (const FString& Requirement : Info->Requires)
            {
                BuildChain(Requirement);
            }
        }
    };
    
    BuildChain(Name);
    return Chain;
}

void UEng_DependencyManager::RegisterCoreDependencies()
{
    // Engine modules
    RegisterDependency(TEXT("Engine"), EEng_DependencyType::Module, TEXT("Engine"));
    RegisterDependency(TEXT("Core"), EEng_DependencyType::Module, TEXT("Core"));
    RegisterDependency(TEXT("CoreUObject"), EEng_DependencyType::Module, TEXT("CoreUObject"));
    RegisterDependency(TEXT("UMG"), EEng_DependencyType::Module, TEXT("UMG"));
    RegisterDependency(TEXT("Slate"), EEng_DependencyType::Module, TEXT("Slate"));
    RegisterDependency(TEXT("SlateCore"), EEng_DependencyType::Module, TEXT("SlateCore"));
    
    // TranspersonalGame module
    RegisterDependency(TEXT("TranspersonalGame"), EEng_DependencyType::Module, TEXT("TranspersonalGame"));
    
    // Add basic relations
    AddDependencyRelation(TEXT("TranspersonalGame"), TEXT("Engine"));
    AddDependencyRelation(TEXT("TranspersonalGame"), TEXT("Core"));
    AddDependencyRelation(TEXT("TranspersonalGame"), TEXT("CoreUObject"));
    AddDependencyRelation(TEXT("Engine"), TEXT("Core"));
    AddDependencyRelation(TEXT("UMG"), TEXT("Slate"));
    AddDependencyRelation(TEXT("Slate"), TEXT("SlateCore"));
}

void UEng_DependencyManager::RegisterModuleDependencies()
{
    // TranspersonalGame classes
    RegisterDependency(TEXT("TranspersonalCharacter"), EEng_DependencyType::Class, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    RegisterDependency(TEXT("TranspersonalGameState"), EEng_DependencyType::Class, TEXT("/Script/TranspersonalGame.TranspersonalGameState"));
    RegisterDependency(TEXT("PCGWorldGenerator"), EEng_DependencyType::Class, TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
    RegisterDependency(TEXT("FoliageManager"), EEng_DependencyType::Class, TEXT("/Script/TranspersonalGame.FoliageManager"));
    
    // Architecture classes
    RegisterDependency(TEXT("Eng_SystemRegistry"), EEng_DependencyType::Class, TEXT("/Script/TranspersonalGame.Eng_SystemRegistry"));
    RegisterDependency(TEXT("Eng_PerformanceMonitor"), EEng_DependencyType::Class, TEXT("/Script/TranspersonalGame.Eng_PerformanceMonitor"));
    RegisterDependency(TEXT("Eng_ModuleValidator"), EEng_DependencyType::Class, TEXT("/Script/TranspersonalGame.Eng_ModuleValidator"));
    
    // Add class dependencies
    AddDependencyRelation(TEXT("TranspersonalCharacter"), TEXT("TranspersonalGame"));
    AddDependencyRelation(TEXT("TranspersonalGameState"), TEXT("TranspersonalGame"));
    AddDependencyRelation(TEXT("PCGWorldGenerator"), TEXT("TranspersonalGame"));
    AddDependencyRelation(TEXT("FoliageManager"), TEXT("TranspersonalGame"));
}

void UEng_DependencyManager::RegisterAssetDependencies()
{
    // Core game assets
    RegisterDependency(TEXT("MinPlayableMap"), EEng_DependencyType::Asset, TEXT("/Game/Maps/MinPlayableMap"));
    RegisterDependency(TEXT("DefaultPawn"), EEng_DependencyType::Asset, TEXT("/Game/Blueprints/DefaultPawn"));
    RegisterDependency(TEXT("GameMode"), EEng_DependencyType::Asset, TEXT("/Game/Blueprints/GameMode"));
    
    // Add asset dependencies
    AddDependencyRelation(TEXT("MinPlayableMap"), TEXT("TranspersonalCharacter"));
    AddDependencyRelation(TEXT("DefaultPawn"), TEXT("TranspersonalCharacter"));
    AddDependencyRelation(TEXT("GameMode"), TEXT("TranspersonalGameState"));
}

EEng_DependencyStatus UEng_DependencyManager::CheckDependencyStatus(const FEng_DependencyInfo& Dependency)
{
    switch (Dependency.Type)
    {
        case EEng_DependencyType::Module:
        {
            if (FModuleManager::Get().IsModuleLoaded(*Dependency.Path))
            {
                return EEng_DependencyStatus::Available;
            }
            return EEng_DependencyStatus::Missing;
        }
        
        case EEng_DependencyType::Class:
        {
            UClass* LoadedClass = LoadClass<UObject>(nullptr, *Dependency.Path);
            if (LoadedClass)
            {
                return EEng_DependencyStatus::Available;
            }
            return EEng_DependencyStatus::Missing;
        }
        
        case EEng_DependencyType::Asset:
        {
            UObject* LoadedAsset = LoadObject<UObject>(nullptr, *Dependency.Path);
            if (LoadedAsset)
            {
                return EEng_DependencyStatus::Available;
            }
            return EEng_DependencyStatus::Missing;
        }
        
        default:
            return EEng_DependencyStatus::Unknown;
    }
}

void UEng_DependencyManager::BuildDependencyGraph()
{
    UE_LOG(LogTemp, Log, TEXT("Building dependency graph..."));
    
    if (DetectCircularDependencies())
    {
        UE_LOG(LogTemp, Error, TEXT("Circular dependencies detected!"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("No circular dependencies found"));
    }
}

bool UEng_DependencyManager::DetectCircularDependencies()
{
    TSet<FString> Visiting;
    TSet<FString> Visited;
    
    TFunction<bool(const FString&)> HasCycle = [&](const FString& Name) -> bool
    {
        if (Visiting.Contains(Name))
        {
            UE_LOG(LogTemp, Error, TEXT("Circular dependency detected at: %s"), *Name);
            return true;
        }
        
        if (Visited.Contains(Name))
        {
            return false;
        }
        
        Visiting.Add(Name);
        
        const FEng_DependencyInfo* Info = Dependencies.Find(Name);
        if (Info)
        {
            for (const FString& Requirement : Info->Requires)
            {
                if (HasCycle(Requirement))
                {
                    return true;
                }
            }
        }
        
        Visiting.Remove(Name);
        Visited.Add(Name);
        return false;
    };
    
    for (const auto& Pair : Dependencies)
    {
        if (!Visited.Contains(Pair.Key))
        {
            if (HasCycle(Pair.Key))
            {
                return true;
            }
        }
    }
    
    return false;
}
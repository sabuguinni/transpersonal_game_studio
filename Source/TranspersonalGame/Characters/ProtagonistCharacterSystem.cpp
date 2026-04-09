#include "ProtagonistCharacterSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

DEFINE_LOG_CATEGORY_STATIC(LogProtagonistSystem, Log, All);

//////////////////////////////////////////////////////////////////////////
// UProtagonistCharacterSubsystem

void UProtagonistCharacterSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogProtagonistSystem, Warning, TEXT("Protagonist Character System Initialized"));
    
    // Initialize default configuration
    CurrentConfiguration = FProtagonistConfiguration();
    CurrentConfiguration.CharacterName = TEXT("Dr. Marcus Chen");
    CurrentConfiguration.ProfessionTitle = TEXT("Paleontologist");
    CurrentConfiguration.Institution = TEXT("University Research Institute");
    CurrentConfiguration.Specialization = TEXT("Mesozoic Era Specialist");
    
    // Setup progression tracking
    bProgressionSystemActive = true;
    
    // Start progression timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ProgressionUpdateTimer,
            this,
            &UProtagonistCharacterSubsystem::UpdateProgression,
            60.0f, // Update every minute
            true
        );
    }
}

void UProtagonistCharacterSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ProgressionUpdateTimer);
    }
    
    bProgressionSystemActive = false;
    Super::Deinitialize();
}

UProtagonistCharacterSubsystem* UProtagonistCharacterSubsystem::Get(const UObject* WorldContext)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        return World->GetSubsystem<UProtagonistCharacterSubsystem>();
    }
    return nullptr;
}

void UProtagonistCharacterSubsystem::SetProtagonistConfiguration(const FProtagonistConfiguration& NewConfiguration)
{
    CurrentConfiguration = NewConfiguration;
    
    UE_LOG(LogProtagonistSystem, Log, TEXT("Protagonist configuration updated: %s"), *NewConfiguration.CharacterName);
    
    // Apply configuration to existing character if present
    if (CurrentProtagonistCharacter.IsValid())
    {
        ApplyConfigurationToCharacter(CurrentProtagonistCharacter.Get(), NewConfiguration);
    }
    
    // Broadcast configuration change
    OnProtagonistConfigurationChanged.Broadcast(NewConfiguration);
}

FProtagonistConfiguration UProtagonistCharacterSubsystem::GetProtagonistConfiguration() const
{
    return CurrentConfiguration;
}

void UProtagonistCharacterSubsystem::RegisterProtagonistCharacter(ACharacter* Character)
{
    if (Character)
    {
        CurrentProtagonistCharacter = Character;
        UE_LOG(LogProtagonistSystem, Log, TEXT("Protagonist character registered: %s"), *Character->GetName());
        
        // Apply current configuration
        ApplyConfigurationToCharacter(Character, CurrentConfiguration);
        
        // Initialize progression
        InitializeCharacterProgression(Character);
    }
}

void UProtagonistCharacterSubsystem::UnregisterProtagonistCharacter()
{
    if (CurrentProtagonistCharacter.IsValid())
    {
        UE_LOG(LogProtagonistSystem, Log, TEXT("Protagonist character unregistered"));
    }
    CurrentProtagonistCharacter.Reset();
}

void UProtagonistCharacterSubsystem::UpdateSurvivalProgression(float DeltaTime)
{
    if (!CurrentProtagonistCharacter.IsValid() || !bProgressionSystemActive)
    {
        return;
    }
    
    FProtagonistSurvivalProgression& Progression = CurrentConfiguration.SurvivalProgression;
    
    // Update time in world
    Progression.DaysInWorld += DeltaTime / (24.0f * 3600.0f); // Convert seconds to days
    
    // Update survival stage based on days
    ESurvivalStage NewStage = CalculateSurvivalStage(Progression.DaysInWorld);
    if (NewStage != Progression.CurrentStage)
    {
        Progression.CurrentStage = NewStage;
        OnSurvivalStageChanged.Broadcast(NewStage);
        UE_LOG(LogProtagonistSystem, Log, TEXT("Survival stage changed to: %d"), (int32)NewStage);
    }
    
    // Update wear and tear
    UpdateWearAndTear(DeltaTime);
    
    // Update physical adaptation
    UpdatePhysicalAdaptation(DeltaTime);
    
    // Apply visual changes
    ApplyProgressionToCharacter(CurrentProtagonistCharacter.Get(), Progression);
}

void UProtagonistCharacterSubsystem::AddSurvivalEvent(const FString& EventType, float Severity)
{
    if (!CurrentProtagonistCharacter.IsValid())
    {
        return;
    }
    
    FProtagonistSurvivalProgression& Progression = CurrentConfiguration.SurvivalProgression;
    
    UE_LOG(LogProtagonistSystem, Log, TEXT("Survival event: %s (Severity: %.2f)"), *EventType, Severity);
    
    // Handle different event types
    if (EventType == TEXT("Injury"))
    {
        Progression.PhysicalCondition = FMath::Max(0.0f, Progression.PhysicalCondition - Severity);
        Progression.CurrentInjuries.Add(FString::Printf(TEXT("Injury_Day%.1f"), Progression.DaysInWorld));
    }
    else if (EventType == TEXT("Stress"))
    {
        Progression.MentalState = FMath::Max(0.0f, Progression.MentalState - Severity);
    }
    else if (EventType == TEXT("ClothingDamage"))
    {
        FProtagonistClothingSystem& Clothing = CurrentConfiguration.ClothingSystem;
        Clothing.ShirtCondition = FMath::Max(0.0f, Clothing.ShirtCondition - Severity);
        Clothing.PantsCondition = FMath::Max(0.0f, Clothing.PantsCondition - Severity);
    }
    else if (EventType == TEXT("Recovery"))
    {
        Progression.PhysicalCondition = FMath::Min(1.0f, Progression.PhysicalCondition + Severity);
        Progression.MentalState = FMath::Min(1.0f, Progression.MentalState + Severity);
    }
    
    // Broadcast event
    OnSurvivalEventOccurred.Broadcast(EventType, Severity);
}

void UProtagonistCharacterSubsystem::SetProgressionSystemActive(bool bActive)
{
    bProgressionSystemActive = bActive;
    UE_LOG(LogProtagonistSystem, Log, TEXT("Progression system %s"), bActive ? TEXT("activated") : TEXT("deactivated"));
}

void UProtagonistCharacterSubsystem::ResetProgression()
{
    if (CurrentProtagonistCharacter.IsValid())
    {
        // Reset to fresh arrival state
        FProtagonistSurvivalProgression& Progression = CurrentConfiguration.SurvivalProgression;
        Progression = FProtagonistSurvivalProgression(); // Reset to defaults
        
        // Reset clothing
        FProtagonistClothingSystem& Clothing = CurrentConfiguration.ClothingSystem;
        Clothing = FProtagonistClothingSystem(); // Reset to defaults
        
        // Apply reset state to character
        ApplyConfigurationToCharacter(CurrentProtagonistCharacter.Get(), CurrentConfiguration);
        
        UE_LOG(LogProtagonistSystem, Log, TEXT("Protagonist progression reset"));
        OnProgressionReset.Broadcast();
    }
}

void UProtagonistCharacterSubsystem::ApplyConfigurationToCharacter(ACharacter* Character, const FProtagonistConfiguration& Configuration)
{
    if (!Character)
    {
        return;
    }
    
    UE_LOG(LogProtagonistSystem, Log, TEXT("Applying configuration to character: %s"), *Configuration.CharacterName);
    
    // Apply physical traits
    ApplyPhysicalTraits(Character, Configuration.PhysicalTraits);
    
    // Apply clothing system
    ApplyClothingSystem(Character, Configuration.ClothingSystem);
    
    // Apply survival progression
    ApplyProgressionToCharacter(Character, Configuration.SurvivalProgression);
}

void UProtagonistCharacterSubsystem::ApplyPhysicalTraits(ACharacter* Character, const FProtagonistPhysicalTraits& Traits)
{
    if (!Character || !Character->GetMesh())
    {
        return;
    }
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    
    // Apply height scaling
    float HeightScale = Traits.Height / 175.0f; // 175cm as base
    Character->SetActorScale3D(FVector(HeightScale, HeightScale, HeightScale));
    
    // Create or update material instances for skin, hair, eyes
    if (UMaterialInterface* BaseMaterial = MeshComp->GetMaterial(0))
    {
        UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, Character);
        if (DynMaterial)
        {
            // Apply skin tone
            DynMaterial->SetVectorParameterValue(TEXT("SkinColor"), FLinearColor::White); // Would map ESkinTone to color
            
            // Apply eye color
            DynMaterial->SetVectorParameterValue(TEXT("EyeColor"), Traits.EyeColor);
            
            // Apply hair color
            DynMaterial->SetVectorParameterValue(TEXT("HairColor"), Traits.HairColor);
            
            MeshComp->SetMaterial(0, DynMaterial);
        }
    }
    
    UE_LOG(LogProtagonistSystem, Log, TEXT("Applied physical traits - Height: %.1fcm, Gender: %d"), 
           Traits.Height, (int32)Traits.Gender);
}

void UProtagonistCharacterSubsystem::ApplyClothingSystem(ACharacter* Character, const FProtagonistClothingSystem& ClothingSystem)
{
    if (!Character)
    {
        return;
    }
    
    // This would integrate with a clothing/equipment system
    // For now, we log the clothing state
    UE_LOG(LogProtagonistSystem, Log, TEXT("Applied clothing system - Base: %s, Shirt: %.2f, Pants: %.2f"), 
           *ClothingSystem.BaseOutfit, ClothingSystem.ShirtCondition, ClothingSystem.PantsCondition);
    
    // In a full implementation, this would:
    // 1. Load appropriate clothing meshes
    // 2. Apply wear/damage materials
    // 3. Handle improvised clothing attachments
    // 4. Update clothing physics properties
}

void UProtagonistCharacterSubsystem::ApplyProgressionToCharacter(ACharacter* Character, const FProtagonistSurvivalProgression& Progression)
{
    if (!Character || !Character->GetMesh())
    {
        return;
    }
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    
    // Apply wear and environmental effects through material parameters
    if (UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(MeshComp->GetMaterial(0)))
    {
        // Dirt accumulation
        DynMaterial->SetScalarParameterValue(TEXT("DirtLevel"), Progression.DirtAccumulation);
        
        // Sun tan
        DynMaterial->SetScalarParameterValue(TEXT("TanLevel"), Progression.SunTanLevel);
        
        // Skin condition
        DynMaterial->SetScalarParameterValue(TEXT("SkinCondition"), Progression.SkinCondition);
        
        // Hair condition
        DynMaterial->SetScalarParameterValue(TEXT("HairCondition"), Progression.HairCondition);
        
        // Beard growth
        if (Progression.bHasBeardGrowth)
        {
            DynMaterial->SetScalarParameterValue(TEXT("BeardGrowth"), Progression.BeardGrowthLevel);
        }
        
        // Muscle definition
        DynMaterial->SetScalarParameterValue(TEXT("MuscleDefinition"), Progression.MuscleDefinition);
    }
    
    UE_LOG(LogProtagonistSystem, Log, TEXT("Applied progression - Days: %.1f, Stage: %d, Condition: %.2f"), 
           Progression.DaysInWorld, (int32)Progression.CurrentStage, Progression.PhysicalCondition);
}

void UProtagonistCharacterSubsystem::InitializeCharacterProgression(ACharacter* Character)
{
    if (!Character)
    {
        return;
    }
    
    // Initialize progression based on preset
    FProtagonistSurvivalProgression& Progression = CurrentConfiguration.SurvivalProgression;
    
    switch (CurrentConfiguration.Preset)
    {
        case EProtagonistPreset::Default_Academic:
            Progression.PhysicalCondition = 0.8f; // Academic, not super fit
            Progression.MentalState = 0.9f; // Intellectually prepared
            break;
            
        case EProtagonistPreset::Rugged_FieldWorker:
            Progression.PhysicalCondition = 1.0f; // Physically fit
            Progression.MentalState = 0.8f; // Experienced but shocked
            break;
            
        case EProtagonistPreset::Young_Graduate:
            Progression.PhysicalCondition = 0.9f; // Young and fit
            Progression.MentalState = 0.6f; // Inexperienced and scared
            break;
            
        case EProtagonistPreset::Experienced_Professor:
            Progression.PhysicalCondition = 0.7f; // Older, less fit
            Progression.MentalState = 1.0f; // Mentally prepared and calm
            break;
    }
    
    UE_LOG(LogProtagonistSystem, Log, TEXT("Character progression initialized for preset: %d"), 
           (int32)CurrentConfiguration.Preset);
}

ESurvivalStage UProtagonistCharacterSubsystem::CalculateSurvivalStage(float DaysInWorld) const
{
    if (DaysInWorld < 1.0f)
        return ESurvivalStage::FreshArrival;
    else if (DaysInWorld < 8.0f)
        return ESurvivalStage::EarlyAdaptation;
    else if (DaysInWorld < 31.0f)
        return ESurvivalStage::SurvivalLearning;
    else if (DaysInWorld < 91.0f)
        return ESurvivalStage::Adaptation;
    else
        return ESurvivalStage::VeteranSurvivor;
}

void UProtagonistCharacterSubsystem::UpdateProgression()
{
    if (CurrentProtagonistCharacter.IsValid() && bProgressionSystemActive)
    {
        // This is called every minute by the timer
        // Update progression with 1 minute of delta time
        UpdateSurvivalProgression(60.0f);
    }
}

void UProtagonistCharacterSubsystem::UpdateWearAndTear(float DeltaTime)
{
    FProtagonistSurvivalProgression& Progression = CurrentConfiguration.SurvivalProgression;
    FProtagonistClothingSystem& Clothing = CurrentConfiguration.ClothingSystem;
    
    // Base wear rate per day
    float BaseWearRate = 0.01f; // 1% per day
    float DailyWear = BaseWearRate * (DeltaTime / (24.0f * 3600.0f));
    
    // Clothing deterioration
    Clothing.ShirtCondition = FMath::Max(0.0f, Clothing.ShirtCondition - DailyWear);
    Clothing.PantsCondition = FMath::Max(0.0f, Clothing.PantsCondition - DailyWear);
    Clothing.BootsCondition = FMath::Max(0.0f, Clothing.BootsCondition - DailyWear * 1.5f); // Boots wear faster
    
    // Hair condition
    Progression.HairCondition = FMath::Max(0.1f, Progression.HairCondition - DailyWear * 0.5f);
    
    // Dirt accumulation
    Progression.DirtAccumulation = FMath::Min(1.0f, Progression.DirtAccumulation + DailyWear * 2.0f);
    
    // Beard growth (if applicable)
    if (CurrentConfiguration.PhysicalTraits.Gender == ECharacterGender::Male)
    {
        if (Progression.DaysInWorld > 2.0f) // Start growing after 2 days
        {
            Progression.bHasBeardGrowth = true;
            Progression.BeardGrowthLevel = FMath::Min(1.0f, (Progression.DaysInWorld - 2.0f) / 30.0f); // Full beard in 30 days
        }
    }
}

void UProtagonistCharacterSubsystem::UpdatePhysicalAdaptation(float DeltaTime)
{
    FProtagonistSurvivalProgression& Progression = CurrentConfiguration.SurvivalProgression;
    
    // Muscle definition increases with survival activities
    float AdaptationRate = 0.005f; // 0.5% per day
    float DailyAdaptation = AdaptationRate * (DeltaTime / (24.0f * 3600.0f));
    
    if (Progression.DaysInWorld > 7.0f) // Start adapting after a week
    {
        Progression.MuscleDefinition = FMath::Min(1.0f, Progression.MuscleDefinition + DailyAdaptation);
    }
    
    // Sun tan development
    Progression.SunTanLevel = FMath::Min(1.0f, Progression.SunTanLevel + DailyAdaptation * 0.5f);
    
    // Mental adaptation
    if (Progression.DaysInWorld > 1.0f)
    {
        // Gradually recover mental state as adaptation occurs
        float MentalRecoveryRate = 0.002f; // 0.2% per day
        float DailyMentalRecovery = MentalRecoveryRate * (DeltaTime / (24.0f * 3600.0f));
        Progression.MentalState = FMath::Min(0.9f, Progression.MentalState + DailyMentalRecovery);
    }
}

//////////////////////////////////////////////////////////////////////////
// ATranspersonalProtagonist

ATranspersonalProtagonist::ATranspersonalProtagonist()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Setup character components
    if (GetMesh())
    {
        GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
        GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    }
    
    // Initialize protagonist data
    ProtagonistConfiguration = FProtagonistConfiguration();
    
    UE_LOG(LogProtagonistSystem, Log, TEXT("Transpersonal Protagonist character created"));
}

void ATranspersonalProtagonist::BeginPlay()
{
    Super::BeginPlay();
    
    // Register with the protagonist system
    if (UProtagonistCharacterSubsystem* ProtagonistSystem = UProtagonistCharacterSubsystem::Get(this))
    {
        ProtagonistSystem->RegisterProtagonistCharacter(this);
    }
    
    UE_LOG(LogProtagonistSystem, Log, TEXT("Protagonist character began play: %s"), 
           *ProtagonistConfiguration.CharacterName);
}

void ATranspersonalProtagonist::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Unregister from the protagonist system
    if (UProtagonistCharacterSubsystem* ProtagonistSystem = UProtagonistCharacterSubsystem::Get(this))
    {
        ProtagonistSystem->UnregisterProtagonistCharacter();
    }
    
    Super::EndPlay(EndPlayReason);
}

void ATranspersonalProtagonist::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update any per-frame protagonist logic here
    // Most progression is handled by the subsystem
}

void ATranspersonalProtagonist::SetProtagonistConfiguration(const FProtagonistConfiguration& NewConfiguration)
{
    ProtagonistConfiguration = NewConfiguration;
    
    // Apply configuration through the subsystem
    if (UProtagonistCharacterSubsystem* ProtagonistSystem = UProtagonistCharacterSubsystem::Get(this))
    {
        ProtagonistSystem->SetProtagonistConfiguration(NewConfiguration);
    }
}

FProtagonistConfiguration ATranspersonalProtagonist::GetProtagonistConfiguration() const
{
    return ProtagonistConfiguration;
}

void ATranspersonalProtagonist::AddSurvivalEvent(const FString& EventType, float Severity)
{
    if (UProtagonistCharacterSubsystem* ProtagonistSystem = UProtagonistCharacterSubsystem::Get(this))
    {
        ProtagonistSystem->AddSurvivalEvent(EventType, Severity);
    }
}

void ATranspersonalProtagonist::ResetProgression()
{
    if (UProtagonistCharacterSubsystem* ProtagonistSystem = UProtagonistCharacterSubsystem::Get(this))
    {
        ProtagonistSystem->ResetProgression();
    }
}

float ATranspersonalProtagonist::GetSurvivalDays() const
{
    return ProtagonistConfiguration.SurvivalProgression.DaysInWorld;
}

ESurvivalStage ATranspersonalProtagonist::GetCurrentSurvivalStage() const
{
    return ProtagonistConfiguration.SurvivalProgression.CurrentStage;
}

float ATranspersonalProtagonist::GetPhysicalCondition() const
{
    return ProtagonistConfiguration.SurvivalProgression.PhysicalCondition;
}

float ATranspersonalProtagonist::GetMentalState() const
{
    return ProtagonistConfiguration.SurvivalProgression.MentalState;
}
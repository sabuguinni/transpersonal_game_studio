#include "Anim_TribalShowcaseManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"

UAnim_TribalShowcaseManager::UAnim_TribalShowcaseManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    AnimationCycleInterval = 5.0f;
    bAutoPlayAnimations = true;
    bRandomizeAnimationTiming = false;
    LastAnimationCycleTime = 0.0f;
    CurrentAnimationIndex = 0;
    
    InitializeTribalShowcaseConfigs();
}

void UAnim_TribalShowcaseManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoPlayAnimations)
    {
        // Start animation cycle after a short delay
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UAnim_TribalShowcaseManager::CreateTribalShowcase, 2.0f, false);
    }
}

void UAnim_TribalShowcaseManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bAutoPlayAnimations && SpawnedTribalCharacters.Num() > 0)
    {
        LastAnimationCycleTime += DeltaTime;
        
        float CycleTime = AnimationCycleInterval;
        if (bRandomizeAnimationTiming)
        {
            CycleTime += FMath::RandRange(-1.0f, 2.0f);
        }
        
        if (LastAnimationCycleTime >= CycleTime)
        {
            CycleTribalAnimations();
            LastAnimationCycleTime = 0.0f;
        }
    }
}

void UAnim_TribalShowcaseManager::CreateTribalShowcase()
{
    UE_LOG(LogTemp, Warning, TEXT("Creating tribal character showcase"));
    
    // Clear existing characters
    SpawnedTribalCharacters.Empty();
    
    // Spawn each tribal character type
    for (const FAnim_TribalShowcaseData& Config : TribalShowcaseConfigs)
    {
        SpawnTribalCharacter(Config);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Tribal showcase created with %d characters"), SpawnedTribalCharacters.Num());
}

void UAnim_TribalShowcaseManager::SpawnTribalCharacter(const FAnim_TribalShowcaseData& ShowcaseData)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot spawn tribal character: World is null"));
        return;
    }
    
    // Spawn TranspersonalCharacter
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    ATranspersonalCharacter* NewCharacter = World->SpawnActor<ATranspersonalCharacter>(
        ATranspersonalCharacter::StaticClass(),
        ShowcaseData.SpawnLocation,
        ShowcaseData.SpawnRotation,
        SpawnParams
    );
    
    if (NewCharacter)
    {
        // Set character name
        NewCharacter->SetActorLabel(ShowcaseData.CharacterName);
        
        // Setup tribal animations
        SetupTribalAnimations(NewCharacter, ShowcaseData.CharacterType);
        
        // Add to spawned characters array
        SpawnedTribalCharacters.Add(NewCharacter);
        
        UE_LOG(LogTemp, Warning, TEXT("Spawned tribal character: %s at location %s"), 
               *ShowcaseData.CharacterName, 
               *ShowcaseData.SpawnLocation.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn tribal character: %s"), *ShowcaseData.CharacterName);
    }
}

void UAnim_TribalShowcaseManager::SetupTribalAnimations(ATranspersonalCharacter* Character, EAnim_TribalCharacterType CharacterType)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot setup tribal animations: Character is null"));
        return;
    }
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot setup tribal animations: Character mesh is null"));
        return;
    }
    
    UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
    if (!AnimInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("Character has no anim instance, using default setup"));
        return;
    }
    
    // Set character-specific animation properties based on type
    switch (CharacterType)
    {
        case EAnim_TribalCharacterType::Warrior:
            // Setup warrior-specific animations (strong, aggressive poses)
            UE_LOG(LogTemp, Log, TEXT("Setting up warrior animations for %s"), *Character->GetName());
            break;
            
        case EAnim_TribalCharacterType::Healer:
            // Setup healer-specific animations (calm, wise gestures)
            UE_LOG(LogTemp, Log, TEXT("Setting up healer animations for %s"), *Character->GetName());
            break;
            
        case EAnim_TribalCharacterType::Hunter:
            // Setup hunter-specific animations (alert, crouched poses)
            UE_LOG(LogTemp, Log, TEXT("Setting up hunter animations for %s"), *Character->GetName());
            break;
            
        case EAnim_TribalCharacterType::Chief:
            // Setup chief-specific animations (commanding, ceremonial poses)
            UE_LOG(LogTemp, Log, TEXT("Setting up chief animations for %s"), *Character->GetName());
            break;
            
        case EAnim_TribalCharacterType::Child:
            // Setup child-specific animations (playful, curious poses)
            UE_LOG(LogTemp, Log, TEXT("Setting up child animations for %s"), *Character->GetName());
            break;
    }
}

void UAnim_TribalShowcaseManager::PlayTribalIdleAnimation(ATranspersonalCharacter* Character, EAnim_TribalCharacterType CharacterType)
{
    if (!Character)
    {
        return;
    }
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp)
    {
        return;
    }
    
    // Play character-type specific idle animation
    UE_LOG(LogTemp, Log, TEXT("Playing idle animation for %s"), *Character->GetName());
}

void UAnim_TribalShowcaseManager::PlayTribalActionAnimation(ATranspersonalCharacter* Character, EAnim_TribalCharacterType CharacterType)
{
    if (!Character)
    {
        return;
    }
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp)
    {
        return;
    }
    
    // Play character-type specific action animation
    UE_LOG(LogTemp, Log, TEXT("Playing action animation for %s"), *Character->GetName());
}

void UAnim_TribalShowcaseManager::CycleTribalAnimations()
{
    if (SpawnedTribalCharacters.Num() == 0)
    {
        return;
    }
    
    // Cycle through different animations for each character
    for (int32 i = 0; i < SpawnedTribalCharacters.Num(); i++)
    {
        ATranspersonalCharacter* Character = SpawnedTribalCharacters[i];
        if (!Character)
        {
            continue;
        }
        
        EAnim_TribalCharacterType CharacterType = static_cast<EAnim_TribalCharacterType>(i % 5);
        
        // Alternate between idle and action animations
        if (CurrentAnimationIndex % 2 == 0)
        {
            PlayTribalIdleAnimation(Character, CharacterType);
        }
        else
        {
            PlayTribalActionAnimation(Character, CharacterType);
        }
    }
    
    CurrentAnimationIndex++;
    UE_LOG(LogTemp, Log, TEXT("Cycled tribal animations (cycle %d)"), CurrentAnimationIndex);
}

TArray<ATranspersonalCharacter*> UAnim_TribalShowcaseManager::GetSpawnedTribalCharacters() const
{
    return SpawnedTribalCharacters;
}

void UAnim_TribalShowcaseManager::InitializeTribalShowcaseConfigs()
{
    TribalShowcaseConfigs.Empty();
    
    TribalShowcaseConfigs.Add(CreateWarriorConfig());
    TribalShowcaseConfigs.Add(CreateHealerConfig());
    TribalShowcaseConfigs.Add(CreateHunterConfig());
    TribalShowcaseConfigs.Add(CreateChiefConfig());
    TribalShowcaseConfigs.Add(CreateChildConfig());
}

FAnim_TribalShowcaseData UAnim_TribalShowcaseManager::CreateWarriorConfig()
{
    FAnim_TribalShowcaseData Config;
    Config.CharacterType = EAnim_TribalCharacterType::Warrior;
    Config.SpawnLocation = FVector(2000, 0, 100);
    Config.SpawnRotation = FRotator(0, 0, 0);
    Config.CharacterName = TEXT("TribalWarrior_Showcase");
    return Config;
}

FAnim_TribalShowcaseData UAnim_TribalShowcaseManager::CreateHealerConfig()
{
    FAnim_TribalShowcaseData Config;
    Config.CharacterType = EAnim_TribalCharacterType::Healer;
    Config.SpawnLocation = FVector(2500, 0, 100);
    Config.SpawnRotation = FRotator(0, 45, 0);
    Config.CharacterName = TEXT("TribalHealer_Showcase");
    return Config;
}

FAnim_TribalShowcaseData UAnim_TribalShowcaseManager::CreateHunterConfig()
{
    FAnim_TribalShowcaseData Config;
    Config.CharacterType = EAnim_TribalCharacterType::Hunter;
    Config.SpawnLocation = FVector(3000, 0, 100);
    Config.SpawnRotation = FRotator(0, 90, 0);
    Config.CharacterName = TEXT("TribalHunter_Showcase");
    return Config;
}

FAnim_TribalShowcaseData UAnim_TribalShowcaseManager::CreateChiefConfig()
{
    FAnim_TribalShowcaseData Config;
    Config.CharacterType = EAnim_TribalCharacterType::Chief;
    Config.SpawnLocation = FVector(3500, 0, 100);
    Config.SpawnRotation = FRotator(0, 135, 0);
    Config.CharacterName = TEXT("TribalChief_Showcase");
    return Config;
}

FAnim_TribalShowcaseData UAnim_TribalShowcaseManager::CreateChildConfig()
{
    FAnim_TribalShowcaseData Config;
    Config.CharacterType = EAnim_TribalCharacterType::Child;
    Config.SpawnLocation = FVector(4000, 0, 100);
    Config.SpawnRotation = FRotator(0, 180, 0);
    Config.CharacterName = TEXT("TribalChild_Showcase");
    return Config;
}